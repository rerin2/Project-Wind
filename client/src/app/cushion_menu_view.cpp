// 이 파일은 무엇을 하나요?
// 쿠션 메뉴와 레벨업 화면 그리기. 자리·색 값의 이유는 docs/VISUALS.md "Cushion menu and level-up" 절에 있어요.

#include "app/cushion_menu_view.hpp"

#include "engine/tween/easing.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/ui/cushion_menu.hpp"

#include "wind/combat/damage.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <string>
#include <string_view>

namespace wind::client {
namespace {

using game::CushionMenu;
using progression::StatId;

constexpr float kAppearSeconds = 0.2f;     // 창이 톡 나타나는 시간 (명세 6.9: 0.15~0.25초)
constexpr float kPadding = 10.0f;          // 창 안쪽 여백
constexpr float kButtonHeight = 18.0f;
constexpr float kButtonGap = 4.0f;
// 메뉴 창: 화면 오른쪽 (가운데의 주인공을 가리지 않게)
constexpr float kMenuWidth = 168.0f;
constexpr float kMenuRight = 24.0f;        // 화면 오른쪽 끝에서 창까지
// 레벨업 창: 오른쪽에 크게
constexpr float kSheetWidth = 300.0f;
constexpr float kSheetHeight = 236.0f;
constexpr float kRowHeight = 20.0f;
constexpr float kNameWidth = 56.0f;        // 능력치 이름 칸
constexpr float kValueWidth = 72.0f;       // "10 → 12" 칸
constexpr float kShakeSeconds = 0.3f;      // 거절 흔들림 시간
constexpr float kShakePixels = 4.0f;       // 흔들림 폭
constexpr float kShakeHz = 18.0f;          // 흔들림 빠르기
constexpr float kDisabledAlpha = 0.4f;     // 흐린 항목
constexpr engine::Color kRaisedColor{142, 220, 128, 255};  // 올린 값 (연두)
constexpr engine::Color kShortColor{230, 96, 84, 255};     // 룬 모자람 (빨강)
constexpr engine::Color kRowFocus{255, 233, 168, 40};      // 고른 줄 배경 (옅은 금빛)

constexpr std::array<std::string_view, CushionMenu::kItemCount> kItemLabels{"레벨업", "마술 칸", "저장", "떠나기"};
constexpr std::array<std::string_view, progression::kStatCount> kStatNames{"생명력", "정신력", "지구력", "힘", "지능"};

// 글자 한 줄 (자르기, 그림자 있음)
void line(engine::UiPainter& painter, engine::RectF box, std::string_view text, engine::Color color,
          engine::TextAlign align = engine::TextAlign::Left, float alpha = 1.0f) {
    engine::TextStyle style;
    style.color = color;
    style.align = align;
    style.max_lines = 1;
    style.wrap = false;
    painter.text(box, text, style, alpha);
}

// 능력치 하나를 올리면 바뀌는 값을 "최대 HP 100 → 118"처럼 (명세 8.3 예시)
std::string preview_text(StatId id, const progression::DerivedStats& now, const progression::DerivedStats& next) {
    switch (id) {
        case StatId::Vigor: return std::format("최대 HP {} → {}", now.max_hp, next.max_hp);
        case StatId::Mind: return std::format("최대 MP {} → {}", now.max_mp, next.max_mp);
        case StatId::Endurance: return std::format("스태미나 {} → {}", now.max_stamina, next.max_stamina);
        case StatId::Strength:
            return std::format("무게 {:.1f} → {:.1f}", now.carry_weight, next.carry_weight);  // 1당 0.6이라 소수 한 자리
        case StatId::Intelligence:
            return std::format("마법 공격력 {:.0f} → {:.0f}", combat::magic_attack_power(combat::kBaseMagicPower, now.intelligence_bonus),
                               combat::magic_attack_power(combat::kBaseMagicPower, next.intelligence_bonus));
    }
    return {};
}

// 창 네모: 톡 나타나기(OutBack)로 살짝 작게 시작해요
engine::RectF pop_rect(float x, float y, float w, float h, float appear) {
    const float scale = 0.9f + 0.1f * engine::ease(engine::Ease::OutBack, appear);
    const float sw = std::round(w * scale);
    const float sh = std::round(h * scale);
    return {std::round(x + (w - sw) * 0.5f), std::round(y + (h - sh) * 0.5f), sw, sh};
}

}  // namespace

void CushionMenuView::update(float dt_seconds, const game::CushionMenu& menu) {
    const int screen = static_cast<int>(menu.screen());
    if (screen != last_screen_) appear_ = 0.0f;  // 메뉴 <-> 레벨업을 오갈 때마다 다시 톡
    last_screen_ = screen;
    appear_ = menu.is_open() ? std::min(1.0f, appear_ + dt_seconds / kAppearSeconds) : 0.0f;
    if (denied_seen_ && menu.denied_count() > *denied_seen_) shake_ = kShakeSeconds;  // 거절되면 흔들기
    denied_seen_ = menu.denied_count();
    shake_ = std::max(0.0f, shake_ - dt_seconds);
}

void CushionMenuView::draw(engine::UiPainter& painter, engine::Vec2 screen, const game::CushionMenu& menu,
                           const game::HeroProgress& progress) const {
    if (menu.screen() == CushionMenu::Screen::Main) draw_main(painter, screen, menu, progress);
    if (menu.screen() == CushionMenu::Screen::LevelUp) draw_level_up(painter, screen, menu, progress);
}

void CushionMenuView::draw_main(engine::UiPainter& painter, engine::Vec2 screen, const game::CushionMenu& menu,
                                const game::HeroProgress& progress) const {
    const auto& theme = painter.theme();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const float height = kPadding * 2.0f + line_h + 6.0f + (kButtonHeight + kButtonGap) * CushionMenu::kItemCount +
                         line_h * 2.0f + 4.0f;
    const engine::RectF rect = pop_rect(screen.x - kMenuWidth - kMenuRight, std::round((screen.y - height) * 0.5f),
                                        kMenuWidth, height, appear_);
    painter.panel(rect, std::min(1.0f, appear_ * 1.5f));
    if (appear_ < 1.0f) return;  // 글은 다 나타난 뒤에
    const float x = rect.x + kPadding;
    const float w = rect.w - kPadding * 2.0f;
    float y = rect.y + kPadding;
    engine::TextStyle title;
    title.font = engine::FontStyle::Bold;
    title.color = theme.text_focused;
    title.align = engine::TextAlign::Center;
    title.max_lines = 1;
    painter.text({x, y, w, line_h}, "캣닢 쿠션", title);
    y += line_h + 6.0f;
    for (std::size_t i = 0; i < CushionMenu::kItemCount; ++i) {
        const auto item = static_cast<CushionMenu::Item>(i);
        const bool enabled = menu.item_enabled(item);
        painter.button({x, y, w, kButtonHeight}, kItemLabels[i], enabled && menu.focused_item() == item,
                       enabled ? 1.0f : kDisabledAlpha);  // 아직 없는 기능은 흐리게
        y += kButtonHeight + kButtonGap;
    }
    y += 4.0f;
    line(painter, {x, y, w, line_h}, std::format("레벨 {}", progress.level()), theme.text_dim);
    line(painter, {x, y + line_h, w, line_h}, std::format("가진 룬 {}", progress.runes()), theme.text);
}

void CushionMenuView::draw_level_up(engine::UiPainter& painter, engine::Vec2 screen, const game::CushionMenu& menu,
                                    const game::HeroProgress& progress) const {
    const auto& theme = painter.theme();
    const auto& sheet = menu.sheet();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const engine::RectF rect = pop_rect(screen.x - kSheetWidth - kMenuRight, std::round((screen.y - kSheetHeight) * 0.5f),
                                        kSheetWidth, kSheetHeight, appear_);
    painter.panel(rect, std::min(1.0f, appear_ * 1.5f));
    if (appear_ < 1.0f) return;
    const float x = rect.x + kPadding;
    const float w = rect.w - kPadding * 2.0f;
    float y = rect.y + kPadding;
    // 1) 제목: 레벨 지금 → 목표
    engine::TextStyle title;
    title.font = engine::FontStyle::Bold;
    title.color = theme.text_focused;
    title.max_lines = 1;
    const bool changed = sheet.changed(progress);
    painter.text({x, y, w, line_h}, changed ? std::format("레벨 {} → {}", progress.level(), sheet.target_level())
                                            : std::format("레벨 {}", progress.level()), title);
    y += line_h + 6.0f;
    // 2) 능력치 다섯 줄: 이름 | 지금 → 목표 | 바뀔 값
    const auto now = progress.derived();
    const auto next = progress.preview(sheet.target());
    for (std::size_t i = 0; i < progression::kStatCount; ++i) {
        const StatId id = progression::kAllStats[i];
        const engine::RectF row{x - 4.0f, y - 2.0f, w + 8.0f, kRowHeight - 2.0f};
        if (sheet.row() == i) painter.renderer().fill_rect(row, kRowFocus);  // 고른 줄
        const int base = progress.stats()[id];
        const int target = sheet.target()[id];
        line(painter, {x, y, kNameWidth, line_h}, kStatNames[i], sheet.row() == i ? theme.text_focused : theme.text);
        const std::string value = target != base ? std::format("{} → {}", base, target) : std::format("{}", base);
        line(painter, {x + kNameWidth, y, kValueWidth, line_h}, value, target != base ? kRaisedColor : theme.text);
        if (target != base) line(painter, {x + kNameWidth + kValueWidth, y, w - kNameWidth - kValueWidth, line_h},
                                 preview_text(id, now, next), theme.text_dim);
        else if (sheet.row() == i) line(painter, {x + kNameWidth + kValueWidth, y, w - kNameWidth - kValueWidth, line_h},
                                        "← → 로 바꾸기", theme.text_dim);
        y += kRowHeight;
    }
    // 3) 비용 줄: 필요한 룬 / 가진 룬 (모자라면 빨강, 거절되면 흔들림)
    y += 4.0f;
    const auto cost = sheet.cost(progress).value_or(0);
    const bool short_of_runes = cost > progress.runes();
    const float shake = shake_ > 0.0f ? std::sin(shake_ * kShakeHz * 6.2832f) * kShakePixels * (shake_ / kShakeSeconds) : 0.0f;
    line(painter, {std::round(x + shake), y, w, line_h}, std::format("필요한 룬 {}   가진 룬 {}", cost, progress.runes()),
         short_of_runes ? kShortColor : theme.text);
    y += line_h + 8.0f;
    // 4) 확정 버튼 (확정할 수 없으면 흐리게) 과 안내
    const bool ready = sheet.can_confirm(progress);
    painter.button({x, y, w, kButtonHeight}, "확정", sheet.row() == game::LevelUpSheet::kConfirmRow,
                   ready ? 1.0f : kDisabledAlpha);
    y += kButtonHeight + 6.0f;
    line(painter, {x, y, w, line_h}, "Enter: 확인   Esc: 돌아가기", theme.text_dim, engine::TextAlign::Center);
}

}  // namespace wind::client
