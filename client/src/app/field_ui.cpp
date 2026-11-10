// 이 파일은 무엇을 하나요?
// HUD 스태미나 바, 지역 이름 배너, UI 시험판을 갱신하고 그리는 곳이에요.
// 색과 크기 값의 이유는 docs/VISUALS.md "HUD and text" 절에 있어요.

#include "app/field_ui.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::client {
namespace {

// 스태미나 바: 화면 왼쪽 위, 테두리 포함 82x7 (안쪽 80x5)
constexpr engine::RectF kStaminaRect{12.0f, 12.0f, 82.0f, 7.0f};
constexpr engine::Color kBarOutline{16, 19, 28, 255};
constexpr engine::Color kBarEmpty{40, 44, 60, 255};
constexpr engine::Color kStaminaColor{126, 206, 92, 255};   // 연두
constexpr engine::Color kStaminaTrail{250, 238, 196, 255};  // 방금 쓴 만큼 (밝은 크림색)
constexpr engine::Color kExhaustedColor{222, 92, 72, 255};  // 지침 (빨강)
constexpr float kExhaustedBlinkHz = 4.0f;                   // 지침일 때 1초에 4번 깜빡

// 배너: 화면 위에서 44픽셀 아래, 이름 양옆으로 짧은 선
constexpr float kBannerY = 44.0f;
constexpr float kBannerLine = 36.0f;    // 양옆 선 길이
constexpr float kBannerGap = 8.0f;      // 이름과 선 사이
constexpr float kBannerMaxWidth = 280.0f;
constexpr engine::Color kBannerBand{0, 0, 0, 96};  // 뒤에 깔리는 반투명 띠

game::DelayedBarTuning stamina_tuning() {
    game::DelayedBarTuning tuning;
    tuning.auto_hide = true;  // 가득 차 있으면 흐려져요 (명세 13.2)
    return tuning;
}

}  // namespace

FieldUi::FieldUi() : stamina_(stamina_tuning()) {}

void FieldUi::enter_area(std::string_view display_name) {
    banner_.show(display_name);
}

UiCommand FieldUi::handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                                bool& consumed_quit) {
    if (bindings.pressed(game::Action::ToggleUiTest, input)) {
        panel_.toggle();
        consumed_quit = false;
        return UiCommand::None;
    }
    return panel_.handle_input(input, bindings, consumed_quit);
}

void FieldUi::update(float dt_seconds, const HudInfo& hud) {
    clock_ += dt_seconds;
    stamina_.set(hud.stamina);
    stamina_.update(dt_seconds);
    exhausted_ = hud.exhausted;
    banner_.update(dt_seconds);
    panel_.update(dt_seconds);
}

void FieldUi::draw_hud(engine::UiPainter& painter, engine::Vec2 screen) {
    draw_stamina(painter);
    draw_banner(painter, screen);
}

void FieldUi::draw_panels(engine::UiPainter& painter, engine::Vec2 screen) {
    panel_.draw(painter, screen);
}

void FieldUi::draw_stamina(engine::UiPainter& painter) {
    auto& renderer = painter.renderer();
    const float alpha = stamina_.alpha();
    const engine::RectF inner{kStaminaRect.x + 1.0f, kStaminaRect.y + 1.0f, kStaminaRect.w - 2.0f,
                              kStaminaRect.h - 2.0f};
    // 지침: 바와 테두리가 빨갛게 깜빡여요 (바가 0이어도 지침인 게 보이게). 보여 주기만 해요.
    const float wave = 0.5f + 0.5f * std::sin(clock_ * kExhaustedBlinkHz * 2.0f * std::numbers::pi_v<float>);
    engine::Color outline = kBarOutline;
    engine::Color fill = kStaminaColor;
    if (exhausted_) {
        outline = kExhaustedColor;
        outline.a = static_cast<std::uint8_t>(110.0f + 145.0f * wave);
        fill = kExhaustedColor;
        fill.a = static_cast<std::uint8_t>(150.0f + 105.0f * wave);
    }
    renderer.fill_rect(kStaminaRect, engine::with_alpha(kBarOutline, alpha));
    renderer.fill_rect(kStaminaRect, engine::with_alpha(outline, alpha));
    renderer.fill_rect(inner, engine::with_alpha(kBarEmpty, alpha));
    // 꼬리(방금 쓴 만큼) 먼저, 그 위에 진짜 바
    const float trail_w = std::round(inner.w * stamina_.trail());
    const float value_w = std::round(inner.w * stamina_.value());
    renderer.fill_rect({inner.x, inner.y, trail_w, inner.h}, engine::with_alpha(kStaminaTrail, alpha));
    renderer.fill_rect({inner.x, inner.y, value_w, inner.h}, engine::with_alpha(fill, alpha));
}

void FieldUi::draw_banner(engine::UiPainter& painter, engine::Vec2 screen) {
    if (!banner_.visible()) {
        return;
    }
    const float alpha = banner_.alpha();
    const auto font = engine::FontStyle::Bold;
    const float text_w = std::min(kBannerMaxWidth, painter.measure(font, banner_.name()));
    const float height = painter.line_height(font);
    const float y = std::round(kBannerY - banner_.rise());
    const float x = std::round((screen.x - text_w) * 0.5f);

    // 뒤에 반투명 띠, 양옆에 짧은 선
    const float band_w = text_w + (kBannerLine + kBannerGap) * 2.0f + 16.0f;
    painter.renderer().fill_rect({std::round((screen.x - band_w) * 0.5f), y - 4.0f, band_w, height + 8.0f},
                                 engine::with_alpha(kBannerBand, alpha));
    const float line_y = y + std::floor(height * 0.5f);
    const auto line_color = engine::with_alpha(painter.theme().text_focused, alpha * 0.8f);
    painter.renderer().fill_rect({x - kBannerGap - kBannerLine, line_y, kBannerLine, 1.0f}, line_color);
    painter.renderer().fill_rect({x + text_w + kBannerGap, line_y, kBannerLine, 1.0f}, line_color);

    engine::TextStyle style;
    style.font = font;
    style.color = painter.theme().text;
    style.align = engine::TextAlign::Center;
    style.max_lines = 1;
    painter.text({x, y, text_w, height}, banner_.name(), style, alpha);
}

}  // namespace wind::client
