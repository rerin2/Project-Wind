// 이 파일은 무엇을 하나요?
// HUD 스태미나 바와 MP 바, 지역 이름 배너, UI 시험판을 갱신하고 그리는 곳이에요.
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

// MP 바: 스태미나 바 2픽셀 아래, 같은 높이. 길이는 최대 MP 1당 0.8픽셀 (스태미나 100 = 80픽셀과 같은 비율)
constexpr float kManaBarY = kStaminaRect.y + kStaminaRect.h + 2.0f;
constexpr float kHudPixelsPerPoint = 0.8f;
constexpr float kManaInnerMin = 8.0f;    // 최대 MP가 아주 작아도 보이게
constexpr float kManaInnerMax = 200.0f;  // 이상한 값이 와도 화면을 넘지 않게
constexpr engine::Color kManaColor{95, 163, 208, 255};  // 하늘색 (팔레트 sky #5fa3d0)
constexpr float kManaBlinkSeconds = 0.6f;  // MP가 모자라 마술이 안 나가면 이만큼 깜빡여요
constexpr float kManaBlinkHz = 8.0f;       // 1초에 8번 (지침 4번보다 빠르게: 순간 알림이라서)

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
    mana_.set(hud.mana);
    mana_.update(dt_seconds);
    mana_max_ = hud.mana_max;
    // "MP 모자람" 횟수가 늘었으면 깜빡이기 시작. 처음 본 값과, 맵을 옮겨 0으로 돌아간 값은 깜빡이지 않아요.
    if (mana_denied_seen_ && hud.mana_denied > *mana_denied_seen_) {
        mana_blink_ = kManaBlinkSeconds;
    }
    mana_denied_seen_ = hud.mana_denied;
    mana_blink_ = std::max(0.0f, mana_blink_ - dt_seconds);
    banner_.update(dt_seconds);
    panel_.update(dt_seconds);
}

void FieldUi::draw_hud(engine::UiPainter& painter, engine::Vec2 screen) {
    draw_stamina(painter);
    draw_mana(painter);
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

// MP 바: 스태미나 바와 같은 모양(테두리, 빈칸, 꼬리, 진짜 바). 흐려지지 않고 늘 보여요.
void FieldUi::draw_mana(engine::UiPainter& painter) {
    auto& renderer = painter.renderer();
    // 안쪽 길이 = 최대 MP × 0.8 (최대 50이면 40픽셀). 능력치가 오르면 바가 길어져요 (명세 13.2)
    const float inner_w = std::round(std::clamp(mana_max_ * kHudPixelsPerPoint, kManaInnerMin, kManaInnerMax));
    const engine::RectF outer{kStaminaRect.x, kManaBarY, inner_w + 2.0f, kStaminaRect.h};
    const engine::RectF inner{outer.x + 1.0f, outer.y + 1.0f, inner_w, outer.h - 2.0f};
    renderer.fill_rect(outer, kBarOutline);  // 테두리
    if (mana_blink_ > 0.0f) {                // MP 모자람: 테두리가 빨갛게 빠르게 깜빡여요
        const float wave = 0.5f + 0.5f * std::sin(clock_ * kManaBlinkHz * 2.0f * std::numbers::pi_v<float>);
        engine::Color warn = kExhaustedColor;
        warn.a = static_cast<std::uint8_t>(80.0f + 175.0f * wave);
        renderer.fill_rect(outer, warn);
    }
    renderer.fill_rect(inner, kBarEmpty);    // 빈칸
    // 꼬리(방금 쓴 만큼) 먼저, 그 위에 진짜 바
    const float trail_w = std::round(inner.w * mana_.trail());
    const float value_w = std::round(inner.w * mana_.value());
    renderer.fill_rect({inner.x, inner.y, trail_w, inner.h}, kStaminaTrail);
    renderer.fill_rect({inner.x, inner.y, value_w, inner.h}, kManaColor);
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
