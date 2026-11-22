// 이 파일은 무엇을 하나요?
// HUD HP·MP·스태미나 바, 지역 이름 배너, UI 시험판을 갱신하고 그리는 곳이에요.
// 색과 크기 값의 이유는 docs/VISUALS.md "HUD and text" 절에 있어요.

#include "app/field_ui.hpp"

#include "game/ui/hud_bar_layout.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <numbers>
#include <optional>
#include <string>

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

// "쓰러졌다" 화면 (12b): 주인공 아래쪽 가로 띠, 그 위에 붉은 글자, 아래에 작은 안내
constexpr std::string_view kFallenText = "쓰러졌다";
constexpr std::string_view kFallenHint = "Enter: 바로 부활";  // 12c: 안 누르면 2.5초 뒤 저절로 부활해요
constexpr float kFallenBandHeight = 48.0f;                    // 띠 높이
// 띠 가운데 높이 (화면 높이의 비율). 카메라가 쓰러진 주인공을 화면 가운데에 두니까, 띠는 그 아래에 둬서 몸을 가리지 않아요.
constexpr float kFallenBandCenter = 0.68f;
constexpr engine::Color kFallenBand{0, 0, 0, 150};            // 띠 색 (흑백 화면 위라 조금 진하게)
constexpr engine::Color kFallenColor{214, 72, 64, 255};       // 글자 (HP 바와 같은 빨강 #d64840)
constexpr float kFallenHintGap = 4.0f;                        // 글자와 안내 사이
constexpr float kFallenTextTop = 8.0f;                        // 띠 위쪽에서 글자까지

// 룬 (13c, 명세 13.2): 오른쪽 아래, 금빛 마름모 + 숫자
constexpr float kRuneMargin = 12.0f;                         // 화면 오른쪽·아래 끝에서
constexpr float kRuneIconSize = 5.0f;                        // 마름모 크기
constexpr float kRuneIconGap = 4.0f;                         // 마름모와 숫자 사이
constexpr engine::Color kRuneColor{255, 214, 120, 255};      // 금빛 (룬 알갱이와 같은 색)
constexpr engine::Color kRuneGainColor{255, 226, 150, 255};  // "+12" (밝은 금빛)
constexpr engine::Color kRuneLossColor{230, 96, 84, 255};    // 잃을 때 (빨강)

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
    hp_max_ = game::smooth_bar_max(hp_max_, hud.hp_max, dt_seconds);  // 레벨업하면 바가 서서히 늘어나요 (13c)
    stamina_.set(hud.stamina);
    stamina_.update(dt_seconds);
    stamina_max_ = game::smooth_bar_max(stamina_max_, hud.stamina_max, dt_seconds);
    exhausted_ = hud.exhausted;
    mana_.set(hud.mana);
    mana_.update(dt_seconds);
    mana_max_ = game::smooth_bar_max(mana_max_, hud.mana_max, dt_seconds);
    // "MP 모자람" 횟수가 늘었으면 깜빡이기 시작. 처음 본 값과, 맵을 옮겨 0으로 돌아간 값은 깜빡이지 않아요.
    if (mana_denied_seen_ && hud.mana_denied > *mana_denied_seen_) {
        mana_blink_ = kManaBlinkSeconds;
    }
    mana_denied_seen_ = hud.mana_denied;
    mana_blink_ = std::max(0.0f, mana_blink_ - dt_seconds);
    banner_.update(dt_seconds);
    fallen_.update(dt_seconds, hud.fallen);
    rest_hud_.update(dt_seconds, hud.rest_prompt && !hud.fallen, hud.rest_denied);
    runes_.update(dt_seconds, hud.runes);  // 룬 숫자 굴리기 (13c)
    lost_runes_ = hud.lost_runes;
    panel_.update(dt_seconds);
}

void FieldUi::draw_hud(engine::UiPainter& painter, engine::Vec2 screen) {
    draw_health(painter);
    draw_stamina(painter);
    draw_mana(painter);
    rest_hud_.draw(painter, screen);  // 쿠션 안내·알림 (배너 아래, 13b)
    draw_runes(painter, screen);      // 오른쪽 아래 룬 숫자 (13c)
    draw_banner(painter, screen);
    draw_fallen(painter, screen);  // HUD 위, 화면 전환 가림막 아래 (부활 페이드와 함께 어두워져요)
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

// "쓰러졌다": 주인공 아래쪽 띠가 진해지고, 붉은 글자가 살짝 올라오며 나타나요. 다 나타나면 안내가 떠요.
void FieldUi::draw_fallen(engine::UiPainter& painter, engine::Vec2 screen) {
    if (!fallen_.visible()) {
        return;
    }
    const float alpha = fallen_.alpha();
    const auto font = engine::FontStyle::Bold;
    const float line = painter.line_height(font);
    const float band_y = std::round(screen.y * kFallenBandCenter - kFallenBandHeight * 0.5f);  // 띠 위쪽 (주인공 아래)
    painter.renderer().fill_rect({0.0f, band_y, screen.x, kFallenBandHeight}, engine::with_alpha(kFallenBand, alpha));

    engine::TextStyle style;
    style.font = font;
    style.color = kFallenColor;
    style.align = engine::TextAlign::Center;
    style.max_lines = 1;
    // 글자는 띠 위쪽 절반 가운데, 나타날 때 아래에서 올라와요
    const float text_y = std::round(band_y + kFallenTextTop + fallen_.drop());
    painter.text({0.0f, text_y, screen.x, line}, kFallenText, style, alpha);

    const float hint = fallen_.hint_alpha();
    if (hint <= 0.0f) {
        return;  // 안내는 글자가 다 나타난 뒤에
    }
    style.font = engine::FontStyle::Regular;
    style.color = painter.theme().text;
    // 잃은 룬 (13c, 명세 13.1)과 부활 안내를 한 줄에
    const std::string hint_line = std::format("잃은 룬 {}   {}", lost_runes_, kFallenHint);
    painter.text({0.0f, text_y + line + kFallenHintGap, screen.x, painter.line_height(style.font)}, hint_line, style,
                 hint);
}

// 오른쪽 아래 룬 (13c, 명세 13.2): 작은 금빛 마름모 + 숫자. 늘면 "+N"이 위로 떠오르고, 잃으면 빨갛게 내려가요.
void FieldUi::draw_runes(engine::UiPainter& painter, engine::Vec2 screen) {
    const auto font = engine::FontStyle::Bold;
    const float line = painter.line_height(font);
    const std::string text = std::to_string(runes_.shown());
    const float text_w = painter.measure(font, text);
    const float x = std::round(screen.x - kRuneMargin - text_w);   // 숫자 왼쪽 끝 (오른쪽 맞춤)
    const float y = std::round(screen.y - kRuneMargin - line);
    // 마름모 아이콘: 가운데가 넓은 가로줄 다섯 개 (진한 테두리 위에 금빛)
    const float icon_x = x - kRuneIconGap - kRuneIconSize;
    const float icon_mid = y + std::floor(line * 0.5f);
    for (int row = -2; row <= 2; ++row) {
        const float half = static_cast<float>(2 - std::abs(row));    // 0,1,2,1,0
        painter.renderer().fill_rect({icon_x + 2.0f - half - 1.0f, icon_mid + row, half * 2.0f + 3.0f, 1.0f},
                                     kBarOutline);
        painter.renderer().fill_rect({icon_x + 2.0f - half, icon_mid + row, half * 2.0f + 1.0f, 1.0f}, kRuneColor);
    }
    engine::TextStyle style;
    style.font = font;
    style.color = runes_.losing() ? kRuneLossColor : painter.theme().text;  // 잃는 중이면 빨강
    style.max_lines = 1;
    style.wrap = false;
    painter.text({x, y, text_w + 1.0f, line}, text, style);
    if (runes_.popup() == 0) return;
    // "+12" / "-6": 숫자 위에서 떠오르며 흐려져요
    const std::string popup = runes_.popup() > 0 ? std::format("+{}", runes_.popup()) : std::format("{}", runes_.popup());
    const float popup_w = painter.measure(font, popup);
    style.color = runes_.popup() > 0 ? kRuneGainColor : kRuneLossColor;
    painter.text({std::round(screen.x - kRuneMargin - popup_w), std::round(y - line - runes_.popup_rise()),
                  popup_w + 1.0f, line}, popup, style, runes_.popup_alpha());
}

}  // namespace wind::client
