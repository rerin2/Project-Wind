// 이 파일은 무엇을 하나요?
// 일시정지 메뉴 창, 탭 줄, 페이지 고르기, 시스템 탭, 키 안내를 그려요.
// 자리·색·시간 값의 이유는 docs/VISUALS.md "Pause menu" 절에 있어요.

#include "app/pause_menu_view.hpp"

#include "app/item_setup.hpp"
#include "app/menu_gear_page.hpp"
#include "app/menu_page.hpp"
#include "app/menu_spell_page.hpp"
#include "app/menu_stats_page.hpp"
#include "app/ui_text.hpp"
#include "engine/render/renderer.hpp"
#include "engine/tween/easing.hpp"
#include "engine/ui/ui_painter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace wind::client {
namespace {

constexpr float kAppearSeconds = 0.18f;   // 나타나는 시간 (명세 6.9: 0.15~0.25초, 14c 가방 화면과 같아요)
constexpr float kTabFadeSeconds = 0.12f;  // 탭을 바꾼 뒤 페이지가 나타나는 시간
constexpr float kTabRise = 4.0f;          // 탭을 바꿀 때 페이지가 올라오는 거리 (픽셀)
constexpr float kSlidePixels = 12.0f;     // 왼쪽에서 미끄러져 들어오는 거리
constexpr float kLeft = 16.0f;            // 화면 왼쪽에서 창까지
constexpr float kPadding = 10.0f;         // 창 안쪽 여백
constexpr float kContentWidth = 196.0f;   // 페이지 너비 (탭 다섯 개가 "능력치"·"시스템"을 자르지 않는 폭)
constexpr float kTabGap = 6.0f;           // 탭 줄과 페이지 사이
constexpr float kSystemButton = 20.0f;    // 시스템 탭 버튼 높이
constexpr float kSystemButtonWidth = 120.0f;
constexpr engine::Color kTabLine{90, 96, 112, 160};  // 탭 줄 아래 가는 줄

constexpr std::array<std::string_view, game::kMenuTabCount> kTabLabels{"가방", "장비", "마술", "능력치", "시스템"};
constexpr std::array<std::string_view, game::kSystemItemCount> kSystemLabels{"설정", "타이틀로", "게임 끄기"};

// 탭마다 맨 아래 키 안내 (창 폭 196픽셀 안에 들어가게 짧게)
std::string_view hint_for(game::MenuTab tab) {
    if (tab == game::MenuTab::Bag) return "R 정렬 · Tab 다음 탭 · Esc 닫기";
    return "Tab 다음 탭 · Esc 닫기";
}

}  // namespace

PauseMenuView::PauseMenuView(engine::TextureHandle icons, engine::TextureHandle spell_icons)
    : bag_view_(icons), icons_(icons), spell_icons_(spell_icons) {}

void PauseMenuView::update(float dt_seconds, const game::PauseMenu& menu) {
    if (!menu.is_open()) {
        appear_ = 0.0f;  // 닫히면 다음에 다시 미끄러져 들어와요
        tab_fade_ = 1.0f;
        shown_tab_ = menu.tab();
        return;
    }
    appear_ = std::min(1.0f, appear_ + dt_seconds / kAppearSeconds);
    if (menu.tab() != shown_tab_) {  // 탭이 바뀌었어요: 페이지를 새로 나타나게
        shown_tab_ = menu.tab();
        tab_fade_ = 0.0f;
    }
    tab_fade_ = std::min(1.0f, tab_fade_ + dt_seconds / kTabFadeSeconds);
}

void PauseMenuView::draw(engine::UiPainter& painter, engine::Vec2 screen, const game::PauseMenu& menu,
                         const ItemSetup& items, const game::HeroProgress& progress) const {
    if (!menu.is_open()) return;
    const auto& theme = painter.theme();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    // 창 크기는 가방 페이지(가장 큰 페이지)에 맞춰 늘 같아요: 탭을 바꿔도 창이 출렁이지 않게
    const float page_h = BagScreenView::size(painter, items.bag).y;
    const float width = kPadding * 2.0f + kContentWidth;
    const float height = kPadding * 2.0f + line_h + 2.0f + kTabGap + page_h + 6.0f + line_h;
    const float eased = engine::ease(engine::Ease::OutCubic, appear_);  // 빨리 들어오다 사뿐히 멈춰요
    const engine::RectF rect{std::round(kLeft - kSlidePixels * (1.0f - eased)), std::round((screen.y - height) * 0.5f),
                             width, height};
    painter.panel(rect, eased);
    const float x = rect.x + kPadding;
    float y = rect.y + kPadding;
    // 1) 탭 줄
    draw_tabs(painter, {x, y, kContentWidth, line_h + 2.0f}, menu.tab(), eased);
    y += line_h + 2.0f + kTabGap;
    // 2) 페이지: 탭을 바꾸면 살짝 올라오며 나타나요
    const float fade = engine::ease(engine::Ease::OutCubic, tab_fade_);
    const float alpha = eased * fade;
    const float rise = std::round(kTabRise * (1.0f - fade));
    const MenuPage page{painter, {x, y + rise, kContentWidth, page_h}, alpha, items, progress, icons_, spell_icons_};
    switch (menu.tab()) {
        case game::MenuTab::Bag: {
            const float bag_w = BagScreenView::size(painter, items.bag).x;
            const engine::Vec2 origin{std::round(x + (kContentWidth - bag_w) * 0.5f), y + rise};  // 칸 묶음은 가운데에
            bag_view_.draw(painter, origin, alpha, appear_ >= 1.0f && tab_fade_ >= 1.0f, menu.bag_screen(), items.bag,
                           items.catalog);
            break;
        }
        case game::MenuTab::Gear: draw_gear_page(page, menu.gear_row()); break;
        case game::MenuTab::Spells: draw_spell_page(page, menu.spell_row()); break;
        case game::MenuTab::Stats: draw_stats_page(page); break;
        case game::MenuTab::System: draw_system(page, menu.system_item()); break;
    }
    y += page_h + 6.0f;
    // 3) 키 안내
    ui_line(painter, {x, y, kContentWidth, line_h}, hint_for(menu.tab()), theme.text_dim, eased,
            engine::TextAlign::Center);
}

void PauseMenuView::draw_tabs(engine::UiPainter& painter, engine::RectF row, game::MenuTab tab, float alpha) const {
    auto& renderer = painter.renderer();
    const auto& theme = painter.theme();
    const float tab_w = row.w / static_cast<float>(game::kMenuTabCount);
    renderer.fill_rect({row.x, row.y + row.h, row.w, 1.0f}, engine::with_alpha(kTabLine, alpha));  // 탭 줄 아래 가는 줄
    for (std::size_t i = 0; i < game::kMenuTabCount; ++i) {
        const bool chosen = static_cast<game::MenuTab>(i) == tab;
        const engine::RectF cell{std::round(row.x + tab_w * static_cast<float>(i)), row.y, std::round(tab_w), row.h - 2.0f};
        ui_line(painter, cell, kTabLabels[i], chosen ? theme.text_focused : theme.text_dim, alpha, engine::TextAlign::Center,
                chosen ? engine::FontStyle::Bold : engine::FontStyle::Regular);
        if (chosen) {  // 고른 탭 밑줄 (금빛 2픽셀, 가는 줄 위에 겹쳐요)
            renderer.fill_rect({cell.x + 4.0f, row.y + row.h - 1.0f, cell.w - 8.0f, 2.0f},
                               engine::with_alpha(theme.text_focused, alpha));
        }
    }
}

void PauseMenuView::draw_system(const MenuPage& page, game::SystemItem focused) {
    const auto& theme = page.painter.theme();
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    float y = page.box.y + 4.0f;
    for (std::size_t i = 0; i < game::kSystemItemCount; ++i) {
        const auto item = static_cast<game::SystemItem>(i);
        const bool enabled = game::PauseMenu::system_enabled(item);
        const engine::RectF button{std::round(page.box.x + (page.box.w - kSystemButtonWidth) * 0.5f), y,
                                   kSystemButtonWidth, kSystemButton};
        page.painter.button(button, kSystemLabels[i], enabled && item == focused,
                            page.alpha * (enabled ? 1.0f : kMenuDisabledAlpha));
        y += kSystemButton + 4.0f;
    }
    y += 6.0f;
    // 저장 안내 (명세 13.1 "저장 안내"): 저장은 쿠션에서만 해요 (명세 12.3)
    ui_line(page.painter, {page.box.x, y, page.box.w, line_h}, "저장은 캣닢 쿠션에서 해요", theme.text_dim, page.alpha,
            engine::TextAlign::Center);
    ui_line(page.painter, {page.box.x, y + line_h, page.box.w, line_h}, "설정 · 타이틀은 준비 중이에요", theme.text_dim,
            page.alpha, engine::TextAlign::Center);
}

}  // namespace wind::client
