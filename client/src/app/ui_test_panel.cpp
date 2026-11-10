// 이 파일은 무엇을 하나요?
// UI 시험판을 열고 닫고, 버튼을 고르고, 그리는 곳이에요.

#include "app/ui_test_panel.hpp"

#include "engine/tween/easing.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace wind::client {
namespace {

// 창 크기와 버튼 배치 (픽셀, 640x360 화면 기준)
constexpr float kPanelWidth = 240.0f;
constexpr float kPanelHeight = 178.0f;
constexpr float kPadding = 12.0f;
constexpr float kButtonHeight = 18.0f;
constexpr float kButtonGap = 4.0f;
// 열릴 때 0.2초 동안 "톡" 나타나요 (명세 6.9: 창 열고 닫을 때 트윈 0.15~0.25초)
constexpr float kAppearSeconds = 0.2f;

struct ButtonDef {
    std::string_view label;
    UiCommand command;
};
constexpr std::array<ButtonDef, 3> kButtons{{
    {"시간 3시간 넘기기", UiCommand::SkipTime},
    {"다음 날씨", UiCommand::NextWeather},
    {"닫기", UiCommand::None},
}};

}  // namespace

UiTestPanel::UiTestPanel() : focus_(kButtons.size()) {}

void UiTestPanel::toggle() {
    open_ = !open_;
    appear_ = 0.0f;
}

UiCommand UiTestPanel::handle_input(const engine::InputState& input,
                                    const game::InputBindings& bindings, bool& consumed_quit) {
    consumed_quit = false;
    if (!open_) {
        return UiCommand::None;
    }
    if (bindings.pressed(game::Action::Quit, input)) {
        consumed_quit = true;
        open_ = false;
        return UiCommand::None;
    }
    if (bindings.pressed(game::Action::MoveUp, input)) {
        focus_.move(-1);
    }
    if (bindings.pressed(game::Action::MoveDown, input)) {
        focus_.move(1);
    }
    if (bindings.pressed(game::Action::Confirm, input)) {
        const UiCommand command = kButtons[focus_.current()].command;
        if (kButtons[focus_.current()].label == "닫기") {
            open_ = false;
        }
        return command;
    }
    return UiCommand::None;
}

void UiTestPanel::update(float dt_seconds) {
    if (open_) {
        appear_ = std::min(1.0f, appear_ + dt_seconds / kAppearSeconds);
    }
}

void UiTestPanel::draw(engine::UiPainter& painter, engine::Vec2 screen) {
    if (!open_) {
        return;
    }
    // 톡 나타나기: 살짝 작게 시작해서 조금 지나쳤다 돌아와요 (OutBack).
    const float grow = engine::ease(engine::Ease::OutBack, appear_);
    const float scale = 0.85f + 0.15f * grow;
    const float alpha = std::min(1.0f, appear_ * 1.5f);
    const float w = std::round(kPanelWidth * scale);
    const float h = std::round(kPanelHeight * scale);
    const engine::RectF rect{std::round((screen.x - w) * 0.5f), std::round((screen.y - h) * 0.5f), w, h};
    painter.panel(rect, alpha);
    if (appear_ < 1.0f) {
        return;  // 글과 버튼은 다 나타난 뒤에 (작을 때 글이 넘치지 않게)
    }

    const auto& theme = painter.theme();
    const float inner_x = rect.x + kPadding;
    const float inner_w = rect.w - kPadding * 2.0f;
    float y = rect.y + kPadding - 2.0f;

    engine::TextStyle title;
    title.font = engine::FontStyle::Bold;
    title.color = theme.text_focused;
    title.align = engine::TextAlign::Center;
    title.max_lines = 1;
    painter.text({inner_x, y, inner_w, painter.line_height(title.font)}, "UI 시험판", title);
    y += painter.line_height(title.font) + 4.0f;

    engine::TextStyle body;
    body.color = theme.text;
    body.max_lines = 3;
    const auto layout = painter.text(
        {inner_x, y, inner_w, painter.line_height(body.font) * 3.0f},
        "한글 글꼴(Galmuri11)로 쓴 글이에요. 상자 폭을 넘으면 띄어쓰기에서 다음 줄로 넘어가요.", body);
    y += layout.height + 4.0f;

    engine::TextStyle clipped;
    clipped.color = theme.text_dim;
    clipped.max_lines = 1;
    painter.text({inner_x, y, inner_w, painter.line_height(clipped.font)},
                 "이 줄은 일부러 아주 길게 써서 상자 끝에서 잘리고 점 세 개가 붙어요", clipped);
    y += painter.line_height(clipped.font) + 8.0f;

    for (std::size_t i = 0; i < kButtons.size(); ++i) {
        painter.button({inner_x, y, inner_w, kButtonHeight}, kButtons[i].label, focus_.current() == i);
        y += kButtonHeight + kButtonGap;
    }
}

}  // namespace wind::client
