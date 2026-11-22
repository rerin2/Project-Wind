// 이 파일은 무엇을 하나요?
// 게임 클라이언트의 심장 박동(게임 루프)이 실제로 뛰는 곳이에요.
// 준비물(창, 그림 관리자, 필드 화면)을 차례로 만들고,
// 게임이 끝날 때까지 "입력 확인 -> 계산 -> 그리기"를 계속 반복해요.

#include "app/client_app.hpp"

#include "app/damage_number_drawing.hpp"
#include "app/debug_overlay.hpp"
#include "app/field_ui.hpp"
#include "app/screenshot_flash.hpp"
#include "app/screenshot_name.hpp"
#include "app/scripted_input.hpp"
#include "app/transition_view.hpp"
#include "app/ui_resources.hpp"
#include "engine/core/frame_clock.hpp"
#include "engine/fx/screen_fade.hpp"
#include "engine/input/input_state.hpp"
#include "engine/platform/sdl_platform.hpp"
#include "engine/render/render_queue.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/input/input_bindings.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/world/game_clock.hpp"
#include "game/world/map_travel.hpp"
#include "game/world/respawn_trigger.hpp"
#include "game/states/field_state.hpp"

#include "wind/log.hpp"
#include "wind/version.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <filesystem>
#include <memory>
#include <optional>
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

// 게임 시계: 실제 60초 = 게임 1시간 (명세 6.4), 시작은 아침 9시, F5를 누르면 3시간 건너뛰기
constexpr float kSecondsPerGameHour = 60.0f;
constexpr float kDefaultStartHour = 9.0f;
constexpr float kSkipHours = 3.0f;
// 맵을 옮길 때 화면이 어두워지는(또는 밝아지는) 시간 (초)
constexpr float kFadeSeconds = 0.35f;
// 부활할 때의 느린 페이드 (어두워지기·밝아지기 각각, 12c, 개발자와 정함): "깨어나는" 느낌
constexpr float kRespawnFadeSeconds = 0.8f;
// 부활 자리: 지금 맵의 기본 등장 자리 (12c, 개발자와 정함. 13단계부터 마지막 쿠션)
constexpr std::string_view kRespawnSpawn = "default";
// UI 테마 파일 (창 테두리, 글꼴, 글자 색)
constexpr std::string_view kUiTheme = "ui/theme.json";
// 스크린샷 폴더 정하기: --screenshot-dir가 있으면 그곳, 없으면 사용자 자료 폴더 안의 screenshots.
// 둘 다 없으면(자료 폴더를 못 찾음) 스크린샷을 찍지 않아요.
std::optional<std::filesystem::path> resolve_screenshot_dir(const ClientOptions& options) {
    if (!options.screenshot_dir.empty()) {
        return options.screenshot_dir;
    }
    const auto user_dir = engine::SdlPlatform::user_data_dir();
    if (user_dir.empty()) {
        return std::nullopt;
    }
    return user_dir / "screenshots";
}

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
    // UI 준비물: 테마·글꼴·테두리 그림. 화가(painter)는 이걸 빌려 쓰니까 먼저 만들어요.
    const UiResources ui_resources = load_ui_resources(resources, platform->text(), kUiTheme);
    engine::UiPainter painter(renderer, platform->text(), ui_resources.theme, ui_resources.frames);
    FieldUi field_ui;  // HUD 스태미나 바, 지역 이름 배너, UI 시험판(F7)
    // 주인공 진행 상태(능력치·룬): 필드 밖에 하나만 두어 맵을 옮기거나 부활해도 남아요 (13a)
    auto progress = game::HeroProgress::load(options_.content_dir);  // 능력치 모두 10, 룬 0
    if (!progress) return 1;  // 성장 규칙 파일이 틀리면 최대 HP 같은 값을 정할 수 없어요 (이유는 로그에)

    // 맵 하나로 필드 화면을 만드는 방법. 처음 시작할 때와 출구로 맵을 옮길 때 같이 써요.
    const game::FieldStress stress{options_.fx_stress, options_.light_stress};
    auto make_field = [&](std::string_view map_path, std::string_view spawn_id) {
        auto made = game::FieldState::create(resources, renderer, map_path, kPlayerSheet,
                                             renderer.logical_size(), *progress, spawn_id, stress,
                                             options_.start_weather);
        if (made) {
            made->set_feel(options_.feel);  // 흔들림·번쩍임 세기 (접근성, 새 맵마다)
        }
        return made;
    };
    auto field = make_field(options_.map, "default");
    if (!field) {
        log::critical("Client cannot start without a valid map; see errors above");
        return 1;  // 맵이 잘못되면 시작할 수 없어요 (자세한 이유는 위쪽 로그에 있어요)
    }
    field_ui.enter_area(field->display_name());  // 첫 지역 이름 배너

    const auto bindings = game::InputBindings::defaults();  // 키 버튼표
    engine::InputState input;                                // 지금 키보드 상태
    engine::FixedStepClock clock(kFixedStepSeconds, kMaxFrameSeconds, kMaxStepsPerFrame);  // 박자 시계
    engine::RenderQueue queue;                               // 그릴 것 목록
    DebugOverlay overlay;                                    // F3 정보판
    const auto screenshot_dir = resolve_screenshot_dir(options_);  // 스크린샷 저장 폴더
    ScreenshotFlash screenshot_flash;                        // 찍은 뒤 "찰칵" 효과
    // 게임 시계: 맵을 옮겨도 이어지도록 필드 화면 밖에 둬요.
    game::GameClock game_clock(options_.start_hour.value_or(kDefaultStartHour), kSecondsPerGameHour);
    game::MapTravel travel(kFadeSeconds);                    // 맵 옮기기 (화면 가리기 -> 새 맵 -> 보여 주기)
    const engine::InputState no_input;                       // 옮기는 중에는 키를 안 누른 셈 쳐요
    std::string current_map = options_.map;                  // 지금 맵 파일 (부활할 맵, 12c)
    game::RespawnTrigger respawn;                            // 쓰러진 뒤 언제 부활할지 (Enter 또는 2.5초, 12c)
    // 출구가 가리키는 맵을 열어요. 못 열면 false (지금 필드는 그대로 둬요).
    // 보통 출구면 주인공 HP·MP·스태미나를 들고 가고, 부활이면 새 필드 그대로(가득 찬 채) 시작해요 (12c).
    const game::MapTravel::EnterMap enter_map = [&](const world::MapExit& exit, bool respawning) {
        const game::HeroVitals carried = field->hero_vitals();  // 떠나기 전 값 (새 필드를 만들기 전에)
        auto next = make_field(exit.to_map, exit.to_spawn);
        if (!next) {
            return false;
        }
        if (!respawning) {
            next->restore_hero(carried);  // 맵을 오가도 HP가 다시 차지 않아요
            log::info("Hero carries hp {}, mp {:.0f}, stamina {:.0f} into the new map", carried.hp, carried.mana,
                      carried.stamina);
        }
        current_map = exit.to_map;
        field = std::move(next);
        field_ui.enter_area(field->display_name());  // 새 지역 이름 배너
        return true;
    };

    auto last_frame = Clock::now();
    // 여기서부터 게임이 끝날 때까지 한 바퀴 = 화면 한 장이에요.
    while (true) {
        // 1~2. 창에서 일어난 일과 키보드 상태를 확인해요.
        input.begin_frame();  // "방금 누름" 기록을 지우고 새 장을 시작
        if (!platform->pump_events(input)) {
            break;  // 창 닫기 버튼을 눌렀어요
        }
        // 시험용 --press: 정해진 장에서 키를 누르고, 다음 장에서 떼요.
        apply_scripted_presses(options_.presses, frames_rendered_ + 1, input);
        apply_scripted_holds(options_.holds, frames_rendered_ + 1, input);
        // UI가 먼저 키를 봐요. 시험판이 열려 있으면 Esc는 게임을 끄지 않고 시험판만 닫아요.
        bool ui_used_quit = false;
        const UiCommand command = field_ui.handle_input(input, bindings, ui_used_quit);
        if (!ui_used_quit && bindings.pressed(game::Action::Quit, input)) {
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
        if (bindings.pressed(game::Action::SkipTime, input) || command == UiCommand::SkipTime) {
            game_clock.skip_hours(kSkipHours);  // F5 (또는 시험판 버튼): 시간 3시간 앞으로
            log::info("Time skipped to {}", game_clock.text());
        }
        if (bindings.pressed(game::Action::NextWeather, input) || command == UiCommand::NextWeather) {
            field->next_weather();  // F6 (또는 시험판 버튼): 다음 날씨
        }

        // 지난 장을 그린 뒤로 시간이 얼마나 흘렀는지 재요.
        const auto now = Clock::now();
        const double frame_seconds = std::chrono::duration<double>(now - last_frame).count();
        last_frame = now;

        // 시험용 --auto-move: 정해진 방향키를 계속 누르고 있는 것처럼 해요.
        if (options_.auto_move) {
            input.set_key(move_key(*options_.auto_move), true);
        }
        // 맵을 옮기는 중(페이드)이거나 UI 창이 열려 있으면 캐릭터가 움직이지 않게 키를 막아요.
        const bool blocked = travel.busy() || field_ui.captures_input();
        const engine::InputState& game_input = blocked ? no_input : input;

        // 3. 누른 키를 필드에 알려 줘요 (구르기 버튼은 메모장에 적어 둬요).
        field->handle_input(game_input, bindings);
        // 4. 박자 시계가 "몇 번 계산해"라고 알려 준 만큼 게임 규칙을 계산해요.
        //    (보통 1번, 컴퓨터가 느려졌으면 2~5번, 아주 빠르면 0번일 수도 있어요)
        const int steps = clock.advance(frame_seconds);
        for (int i = 0; i < steps; ++i) {
            field->fixed_update(clock.step_seconds(), game_input, bindings);
            game_clock.advance(clock.step_seconds());
        }
        // 5. 그림 넘기기, 먼지·잔상, 카메라는 화면 한 장마다 해요.
        //    alpha: 지난 박자와 다음 박자 사이 어디쯤인지 -> 위치를 부드럽게 이어 그려요.
        field->frame_update(frame_seconds, clock.alpha(), game_clock.hour());
        // 부활할 때인가요? 쓰러짐 화면이 다 나타난 뒤 Enter 또는 2.5초 (맵을 옮기는 중에는 세지 않아요, 12c)
        const bool respawn_now =
            respawn.update(static_cast<float>(frame_seconds), field->hero_down(),
                           field_ui.fallen_ready() && !travel.busy(),
                           bindings.pressed(game::Action::Confirm, game_input));
        // 출구를 밟았으면 맵 옮기기를 시작하고, 화면이 다 가려지면 새 맵을 열어요.
        if (!travel.busy()) {
            if (auto exit = field->take_exit_request()) {
                travel.begin(std::move(*exit));
            } else if (respawn_now) {
                // 부활: 느린 페이드 -> 지금 맵을 새로 읽어 기본 자리에서 시작 (명세 9.5, 12c).
                // 새 필드라 HP·MP·스태미나가 가득 차고, 쓰러뜨린 몬스터도 다시 나와요.
                world::MapExit home;
                home.to_map = current_map;
                home.to_spawn = std::string(kRespawnSpawn);
                home.transition = world::ScreenTransition::Fade;
                travel.begin(std::move(home), {true, kRespawnFadeSeconds});
            }
        }
        travel.update(static_cast<float>(frame_seconds), enter_map);
        // HUD 바에 지금 값(0~1)과 최대값(바 길이용)을 알려 줘요. 위에서부터 HP·MP·스태미나.
        HudInfo hud;
        hud.hp = field->health().fraction();
        hud.hp_max = static_cast<float>(field->health().max());
        hud.stamina = field->stamina().fraction();
        hud.stamina_max = field->stamina().max();
        hud.exhausted = field->stamina().exhausted();
        hud.mana = field->mana().fraction();
        hud.mana_max = field->mana().max();
        hud.mana_denied = field->mana_denied_count();
        hud.fallen = field->hero_down();  // 쓰러지면 "쓰러졌다" 화면 (12b), 부활 안내 (12c)
        field_ui.update(static_cast<float>(frame_seconds), hud);

        // 6. 그리기: 그릴 것을 목록에 모으고, 순서대로 줄 세운 다음, 한 번에 그려요.
        queue.clear();
        field->render(queue);
        queue.sort();
        renderer.begin_frame(kClearColor);  // 바탕색으로 화면 지우기
        platform->text().begin_frame();
        painter.begin_frame();
        // 월드(땅 ~ 공중 효과)를 먼저 그리고, 그 위에 조명(어둠과 빛)을 입힌 다음,
        // 조명의 영향을 받으면 안 되는 것(스스로 빛나는 것, 전경, 충돌 표시)을 그려요.
        renderer.submit(queue, engine::DrawLayer::Ground, engine::DrawLayer::WorldFx);
        renderer.apply_lighting(field->lighting());
        renderer.submit(queue, engine::DrawLayer::Glow, engine::DrawLayer::DebugOverlay);
        // 피해 숫자: 조명 뒤라 밤에도 잘 보이고, HUD보다 아래예요 (10b)
        draw_damage_numbers(painter, field->camera(), field->damage_numbers());
        // 쓰러졌으면 지금까지 그린 월드를 흑백으로 (12b). HUD와 "쓰러졌다" 글자는 이 뒤라 색이 남아요.
        renderer.apply_desaturation(field->desaturation());
        // 7. HUD(스태미나 바)와 지역 이름 배너. 화면 가리기보다 아래라서 맵과 함께 어두워져요.
        const engine::Vec2 screen = renderer.logical_size();
        field_ui.draw_hud(painter, screen);
        // 8. 맵을 옮기는 중이면 화면을 검게 가려요 (디버그 정보판은 그 위에 보여요).
        //    아이리스는 주인공을 가운데 두고 오므라들어요. 못 간 출구가 있으면 잠깐 알려 줘요.
        draw_transition(renderer, travel.fade(), field->focus_on_screen());
        draw_notice(painter, travel.notice());
        field_ui.draw_panels(painter, screen);  // UI 시험판 (F7)
        // 9. 디버그 정보판은 맵 위에 맨 나중에 그려요 (그래야 가려지지 않아요).
        overlay.record_frame(frame_seconds);
        const std::string ui_line =
            std::format("ui text draws {}  cached {}  clipped {}", platform->text().draws_this_frame(),
                        platform->text().cached(), painter.clipped_this_frame());
        overlay.draw(renderer, field->debug_info(), resources.stats(), clock.dropped_frames(),
                     game_clock.text(), ui_line);
        // 지난번 "찰칵" 효과가 아직 남아 있으면 맨 위에 그려요.
        screenshot_flash.update(static_cast<float>(frame_seconds));
        screenshot_flash.draw(renderer);
        // F12를 눌렀거나 --screenshot-at 장 번호가 되면, 이번 화면을 그림 파일로 저장해요.
        const bool timed_shot = std::ranges::find(options_.screenshot_at, frames_rendered_ + 1) !=
                                options_.screenshot_at.end();
        const bool shot = bindings.pressed(game::Action::Screenshot, input) || timed_shot;
        if (shot && screenshot_dir) {
            renderer.request_screenshot(*screenshot_dir / screenshot_file_name(local_now()));
        } else if (shot) {
            log::warn("Screenshot skipped: no screenshot folder is available");
        }
        renderer.end_frame();  // 다 그린 그림을 화면에 "짠" 하고 보여 줘요
        if (shot && screenshot_dir) {
            screenshot_flash.trigger();  // 사진은 이미 찍혔으니, 다음 화면부터 번쩍여요
        }
        ++frames_rendered_;
    }

    log::info("Client stopping after {} frames", frames_rendered_);
    field.reset();  // 필드 화면부터 정리
    log::info("Client stopped cleanly");
    return 0;
}

}  // namespace wind::client
