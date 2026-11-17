// 이 파일은 무엇을 하나요?
// HUD HP·MP·스태미나 바, 지역 이름 배너, UI 시험판을 갱신하고 그리는 곳이에요.
// 색과 크기 값의 이유는 docs/VISUALS.md "HUD and text" 절에 있어요.

#include "app/field_ui.hpp"

#include "game/ui/hud_bar_layout.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <optional>

namespace wind::client {
namespace {

// 바 세 줄의 자리와 길이는 game/ui/hud_bar_layout.hpp가 계산해요 (위에서부터 HP·MP·스태미나).
constexpr engine::Color kBarOutline{16, 19, 28, 255};
constexpr engine::Color kBarEmpty{40, 44, 60, 255};
constexpr engine::Color kBarTrail{250, 238, 196, 255};  // 방금 줄어든 만큼 (밝은 크림색, 세 바 공통)
constexpr engine::Color kHealthColor{214, 72, 64, 255};  // 빨강 (허수아비·슬라임 HP 바와 같은 #d64840)
constexpr engine::Color kStaminaColor{126, 206, 92, 255};   // 연두
constexpr engine::Color kExhaustedColor{222, 92, 72, 255};  // 지침 (빨강)
constexpr float kExhaustedBlinkHz = 4.0f;                   // 지침일 때 1초에 4번 깜빡
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

// 바 한 줄의 색 (warn이 있으면 테두리 위에 덧칠해 깜빡임을 보여 줘요)
struct BarLook {
    engine::Color fill;                 // 진짜 바 색
    std::optional<engine::Color> warn;  // 경고 테두리 (지침, MP 모자람). 없으면 보통 테두리
    float alpha = 1.0f;                 // 바 전체 투명도 (스태미나 자동 숨김)
};

// 바 한 줄을 그려요: 테두리 → 경고 테두리 → 빈칸 → 꼬리 → 진짜 바. 세 바가 모두 이걸 써요.
// row: 몇 번째 줄인지, max_points: 최대값 (바 길이가 여기에 비례해요)
void draw_bar(engine::Renderer& renderer, game::HudBarRow row, float max_points, const game::DelayedBar& bar,
              const BarLook& look) {
    const float inner_w = game::hud_bar_inner_width(max_points);  // 최대 100이면 80픽셀
    const engine::RectF outer{game::kHudBarLeft, game::hud_bar_y(row), inner_w + 2.0f, game::kHudBarHeight};
    const engine::RectF inner{outer.x + 1.0f, outer.y + 1.0f, inner_w, outer.h - 2.0f};
    renderer.fill_rect(outer, engine::with_alpha(kBarOutline, look.alpha));  // 테두리
    if (look.warn) {
        renderer.fill_rect(outer, engine::with_alpha(*look.warn, look.alpha));  // 깜빡이는 경고 테두리
    }
    renderer.fill_rect(inner, engine::with_alpha(kBarEmpty, look.alpha));  // 빈칸
    // 꼬리(방금 줄어든 만큼) 먼저, 그 위에 진짜 바. 반 픽셀이 생기지 않게 반올림해요.
    const float trail_w = std::round(inner.w * bar.trail());
    const float value_w = std::round(inner.w * bar.value());
    renderer.fill_rect({inner.x, inner.y, trail_w, inner.h}, engine::with_alpha(kBarTrail, look.alpha));
    renderer.fill_rect({inner.x, inner.y, value_w, inner.h}, engine::with_alpha(look.fill, look.alpha));
}

// 0~1을 오가는 사인 물결 (깜빡임용). hz: 1초에 몇 번
float blink_wave(float clock, float hz) {
    return 0.5f + 0.5f * std::sin(clock * hz * 2.0f * std::numbers::pi_v<float>);
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
    health_.set(hud.hp);
    health_.update(dt_seconds);
    hp_max_ = hud.hp_max;
    stamina_.set(hud.stamina);
    stamina_.update(dt_seconds);
    stamina_max_ = hud.stamina_max;
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
    draw_health(painter);
    draw_stamina(painter);
    draw_mana(painter);
    draw_banner(painter, screen);
}

void FieldUi::draw_panels(engine::UiPainter& painter, engine::Vec2 screen) {
    panel_.draw(painter, screen);
}

// HP 바: 맨 위 줄, 빨강. 흐려지지 않고 늘 보여요 (12a).
void FieldUi::draw_health(engine::UiPainter& painter) {
    draw_bar(painter.renderer(), game::HudBarRow::Health, hp_max_, health_, {kHealthColor, std::nullopt});
}

// 스태미나 바: 맨 아래 줄. 가득 차면 흐려지고, 지치면 바와 테두리가 빨갛게 깜빡여요.
void FieldUi::draw_stamina(engine::UiPainter& painter) {
    BarLook look{kStaminaColor, std::nullopt, stamina_.alpha()};
    if (exhausted_) {  // 바가 0이어도 지침인 게 보이게 테두리까지 깜빡여요. 보여 주기만 해요.
        const float wave = blink_wave(clock_, kExhaustedBlinkHz);
        engine::Color outline = kExhaustedColor;
        outline.a = static_cast<std::uint8_t>(110.0f + 145.0f * wave);
        look.warn = outline;
        look.fill = kExhaustedColor;
        look.fill.a = static_cast<std::uint8_t>(150.0f + 105.0f * wave);
    }
    draw_bar(painter.renderer(), game::HudBarRow::Stamina, stamina_max_, stamina_, look);
}

// MP 바: 가운데 줄. 흐려지지 않고 늘 보여요. MP가 모자라 마술이 안 나가면 테두리가 빨갛게 빠르게 깜빡여요.
void FieldUi::draw_mana(engine::UiPainter& painter) {
    BarLook look{kManaColor, std::nullopt};
    if (mana_blink_ > 0.0f) {
        engine::Color warn = kExhaustedColor;
        warn.a = static_cast<std::uint8_t>(80.0f + 175.0f * blink_wave(clock_, kManaBlinkHz));
        look.warn = warn;
    }
    draw_bar(painter.renderer(), game::HudBarRow::Mana, mana_max_, mana_, look);
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
