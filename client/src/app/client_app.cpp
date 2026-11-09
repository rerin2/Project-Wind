// 이 파일은 무엇을 하나요?
// 게임 클라이언트의 심장 박동(게임 루프)이 실제로 뛰는 곳이에요.
// 준비물(창, 그림 관리자, 필드 화면)을 차례로 만들고,
// 게임이 끝날 때까지 "입력 확인 -> 계산 -> 그리기"를 계속 반복해요.

#include "app/client_app.hpp"

#include "app/debug_overlay.hpp"
#include "engine/core/frame_clock.hpp"
#include "engine/input/input_state.hpp"
#include "engine/platform/sdl_platform.hpp"
#include "engine/render/render_queue.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/input/input_bindings.hpp"
#include "game/states/field_state.hpp"

#include "wind/log.hpp"
#include "wind/version.hpp"

#include <chrono>
#include <string>

namespace wind::client {
namespace {

// 스톱워치: 컴퓨터 시계를 바꿔도 거꾸로 가지 않는 시계예요.
using Clock = std::chrono::steady_clock;

// 게임 규칙을 계산하는 박자: 1초에 60번. 화면을 몇 장 그리는지(FPS)와는 따로예요.
// 이동이나 (나중의) 전투 계산은 이 박자로만 해요.
constexpr double kFixedStepSeconds = 1.0 / 60.0;
// 컴퓨터가 잠깐 멈췄을 때를 대비한 한계:
// 한 번에 0.25초까지만 인정하고, 한 번에 최대 5박자까지만 몰아서 계산해요.
constexpr double kMaxFrameSeconds = 0.25;
constexpr int kMaxStepsPerFrame = 5;
// 게임 화면의 기본 크기(논리 해상도) 640x360. 32픽셀 칸이 가로 20칸, 세로 약 11칸 보여요.
// 창이 크면 2배, 3배처럼 딱 떨어지는 배수로 키워서 도트 그림이 흐려지지 않아요.
constexpr std::int32_t kLogicalWidth = 640;
constexpr std::int32_t kLogicalHeight = 360;
// 플레이어 캐릭터 그림 설명서 위치 (콘텐츠 폴더 기준)
constexpr std::string_view kPlayerSheet = "sprites/test_hero.json";
// 화면을 지울 때 칠하는 바탕색 (아주 어두운 남색)
constexpr engine::Color kClearColor{12, 16, 24, 255};

}  // namespace

ClientApp::ClientApp(ClientOptions options) : options_(std::move(options)) {}

int ClientApp::run() {
    log::info("{} client {} starting", kGameName, kVersion);
    log::info("Content folder: {}", options_.content_dir.string());

    // 준비물을 만드는 순서가 중요해요. C++에서는 나중에 만든 것이 먼저 정리돼요.
    // (쌓은 접시를 위에서부터 치우는 것과 같아요.)
    // 그래서 필드 -> 그림 관리자 -> 창·렌더러 순서로 안전하게 치워져요.
    // 그림 관리자가 렌더러보다 먼저 치워져야, 그림을 지울 때 렌더러가 아직 살아 있어요.
    auto platform = engine::SdlPlatform::create({
        .title = std::string(kGameName),
        .width = options_.window_width,
        .height = options_.window_height,
        .logical_width = kLogicalWidth,
        .logical_height = kLogicalHeight,
    });
    if (!platform) {
        log::critical("Client failed to initialize the platform layer");
        return 1;  // 창을 못 만들면 게임을 할 수 없어요
    }
    engine::Renderer& renderer = platform->renderer();           // 그림 그리는 담당
    engine::ResourceManager resources(renderer, options_.content_dir);  // 그림 파일 보관 담당

    auto field = game::FieldState::create(resources, renderer, options_.map, kPlayerSheet,
                                          renderer.logical_size());
    if (!field) {
        log::critical("Client cannot start without a valid map; see errors above");
        return 1;  // 맵이 잘못되면 시작할 수 없어요 (자세한 이유는 위쪽 로그에 있어요)
    }

    const auto bindings = game::InputBindings::defaults();  // 키 버튼표
    engine::InputState input;                                // 지금 키보드 상태
    engine::FixedStepClock clock(kFixedStepSeconds, kMaxFrameSeconds, kMaxStepsPerFrame);  // 박자 시계
    engine::RenderQueue queue;                               // 그릴 것 목록
    DebugOverlay overlay;                                    // F3 정보판

    auto last_frame = Clock::now();
    // 여기서부터 게임이 끝날 때까지 한 바퀴 = 화면 한 장이에요.
    while (true) {
        // 1~2. 창에서 일어난 일과 키보드 상태를 확인해요.
        input.begin_frame();  // "방금 누름" 기록을 지우고 새 장을 시작
        if (!platform->pump_events(input)) {
            break;  // 창 닫기 버튼을 눌렀어요
        }
        if (bindings.pressed(game::Action::Quit, input)) {
            log::info("Quit requested");
            break;  // Esc
        }
        if (options_.max_frames && frames_rendered_ >= *options_.max_frames) {
            log::info("Reached --max-frames {}", *options_.max_frames);
            break;  // 시험용: 정해진 장수를 다 그렸어요
        }
        if (bindings.pressed(game::Action::ToggleDebug, input)) {
            overlay.toggle();  // F3
        }

        // 지난 장을 그린 뒤로 시간이 얼마나 흘렀는지 재요.
        const auto now = Clock::now();
        const double frame_seconds = std::chrono::duration<double>(now - last_frame).count();
        last_frame = now;

        // 3. 누른 키를 필드에 알려 줘요 (구르기 버튼은 메모장에 적어 둬요).
        field->handle_input(input, bindings);
        // 4. 박자 시계가 "몇 번 계산해"라고 알려 준 만큼 게임 규칙을 계산해요.
        //    (보통 1번, 컴퓨터가 느려졌으면 2~5번, 아주 빠르면 0번일 수도 있어요)
        const int steps = clock.advance(frame_seconds);
        for (int i = 0; i < steps; ++i) {
            field->fixed_update(clock.step_seconds(), input, bindings);
        }
        // 5. 그림 넘기기, 먼지·잔상, 카메라는 화면 한 장마다 해요.
        //    alpha: 지난 박자와 다음 박자 사이 어디쯤인지 -> 위치를 부드럽게 이어 그려요.
        field->frame_update(frame_seconds, clock.alpha());

        // 6. 그리기: 그릴 것을 목록에 모으고, 순서대로 줄 세운 다음, 한 번에 그려요.
        queue.clear();
        field->render(queue);
        queue.sort();
        renderer.begin_frame(kClearColor);  // 바탕색으로 화면 지우기
        renderer.submit(queue);
        // 8. 디버그 정보판은 맵 위에 맨 나중에 그려요 (그래야 가려지지 않아요).
        overlay.record_frame(frame_seconds);
        overlay.draw(renderer, field->debug_info(), resources.stats(), clock.dropped_frames());
        renderer.end_frame();  // 다 그린 그림을 화면에 "짠" 하고 보여 줘요
        ++frames_rendered_;
    }

    log::info("Client stopping after {} frames", frames_rendered_);
    field.reset();  // 필드 화면부터 정리
    log::info("Client stopped cleanly");
    return 0;
}

}  // namespace wind::client
