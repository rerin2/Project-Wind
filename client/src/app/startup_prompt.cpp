// 이 파일은 무엇을 하나요?
// 시작 질문 창의 화면 반복과 그리기예요 (startup_prompt.hpp 설명 참고).

#include "app/startup_prompt.hpp"

#include "app/client_options.hpp"
#include "app/screenshot_name.hpp"
#include "app/scripted_input.hpp"
#include "app/ui_text.hpp"
#include "engine/input/input_state.hpp"
#include "engine/platform/sdl_platform.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/input/input_bindings.hpp"

#include "wind/log.hpp"

#include <algorithm>
#include <string_view>

namespace wind::client {
namespace {

constexpr engine::Color kBackground{12, 16, 24, 255};       // 게임 바탕과 같은 아주 어두운 남색
constexpr engine::Color kTitleColor{255, 233, 168, 255};    // 제목 (옅은 금빛)
constexpr engine::Color kTextColor{230, 230, 236, 255};     // 설명
constexpr engine::Color kKeyColor{170, 200, 255, 255};      // 키 안내 (옅은 하늘색)
constexpr float kPanelWidth = 320.0f;
constexpr float kPanelHeight = 112.0f;
constexpr float kPadding = 14.0f;
constexpr float kLineGap = 18.0f;                           // 줄 간격 (Galmuri 12px + 여유)

// 창 하나 그리기: 제목, 설명 한 줄, 키 안내 한두 줄
void draw_question(const game::StartupQuestion& question, engine::UiPainter& painter) {
    const engine::Vec2 screen = painter.renderer().logical_size();
    const engine::RectF panel{(screen.x - kPanelWidth) * 0.5f, (screen.y - kPanelHeight) * 0.5f, kPanelWidth,
                              kPanelHeight};
    painter.panel(panel);
    const float x = panel.x + kPadding;
    const float w = panel.w - kPadding * 2.0f;
    float y = panel.y + kPadding;
    const bool too_new = question.kind() == game::StartupQuestion::Kind::TooNew;
    ui_line(painter, {x, y, w, kLineGap}, too_new ? "더 새로운 버전의 저장이다" : "저장 파일이 손상되었다", kTitleColor);
    y += kLineGap * 1.5f;
    ui_line(painter, {x, y, w, kLineGap},
            too_new ? "이 버전에서는 불러올 수 없다" : "손상된 파일은 따로 보관한다", kTextColor);
    y += kLineGap * 1.5f;
    if (too_new) {
        ui_line(painter, {x, y, w, kLineGap}, "Enter 게임 끄기", kKeyColor);
    } else if (question.offers_backup()) {
        ui_line(painter, {x, y, w, kLineGap}, "Enter 백업 불러오기", kKeyColor);
        ui_line(painter, {x, y + kLineGap, w, kLineGap}, "Esc 새로 시작", kKeyColor);
    } else {
        ui_line(painter, {x, y, w, kLineGap}, "Enter 새로 시작", kKeyColor);
    }
}

}  // namespace

game::StartupQuestion::Answer ask_startup_question(const game::StartupQuestion& question, engine::SdlPlatform& platform,
                                                   engine::UiPainter& painter, const game::InputBindings& bindings,
                                                   const ClientOptions& options, std::uint64_t& frame) {
    engine::InputState input;
    while (true) {
        input.begin_frame();
        if (!platform.pump_events(input)) return game::StartupQuestion::Answer::Quit;  // 창 닫기
        apply_scripted_presses(options.presses, frame + 1, input);  // 시험용 --press enter@N
        if (options.max_frames && frame >= *options.max_frames) return game::StartupQuestion::Answer::Quit;
        const auto answer = question.answer(bindings.pressed(game::Action::Confirm, input),
                                            bindings.pressed(game::Action::Back, input));
        if (answer) return *answer;
        // 그리기 (조명·필드 없이 창만)
        engine::Renderer& renderer = platform.renderer();
        renderer.begin_frame(kBackground);
        platform.text().begin_frame();
        painter.begin_frame();
        draw_question(question, painter);
        // 시험용 --screenshot-at (화면 확인): --screenshot-dir를 준 실행에서만 찍어요
        const bool shot = std::ranges::find(options.screenshot_at, frame + 1) != options.screenshot_at.end();
        if (shot && !options.screenshot_dir.empty()) {
            renderer.request_screenshot(options.screenshot_dir / screenshot_file_name(local_now()));
        }
        renderer.end_frame();
        ++frame;
    }
}

}  // namespace wind::client
