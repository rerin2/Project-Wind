// 이 파일은 무엇을 하나요?
// "마술" 탭: 마술 칸, 배운 마술 목록, 고른 마술 설명을 그려요. 자리 값의 이유는 docs/VISUALS.md "Pause menu" 절.

#include "app/menu_spell_page.hpp"

#include "app/item_setup.hpp"
#include "app/menu_page.hpp"
#include "app/ui_text.hpp"
#include "engine/render/renderer.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/progression/spell_book.hpp"

#include <algorithm>
#include <format>
#include <string>

namespace wind::client {
namespace {

constexpr float kSlotCell = 22.0f;    // 마술 칸 크기
constexpr float kSlotGap = 4.0f;      // 칸 사이
constexpr float kListRow = 18.0f;     // 배운 마술 한 줄 높이
constexpr std::size_t kListRows = 5;  // 목록 자리 (마술 다섯 개가 다 보여요)
constexpr engine::Color kSelectedSlot{255, 204, 92, 255};  // 고른 칸 테두리 (HUD 칸 점과 같은 진한 금빛)

// 동작 이름
std::string_view motion_name(combat::SpellMotion motion) {
    switch (motion) {
        case combat::SpellMotion::Projectile: return "빛 탄";
        case combat::SpellMotion::Melee: return "근접";
        case combat::SpellMotion::Beam: return "광선 (누르는 동안)";
    }
    return {};
}

// 이 마술이 든 칸 번호 (1부터, 없으면 0)
std::size_t slot_of(const progression::SpellBook& book, std::string_view id, std::size_t open) {
    for (std::size_t i = 0; i < open; ++i) {
        if (book.in_slot(i, open) == id) return i + 1;
    }
    return 0;
}

// 1) 마술 칸 줄
float draw_slots(const MenuPage& page, float y, std::size_t open) {
    auto& renderer = page.painter.renderer();
    const auto& theme = page.painter.theme();
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    const auto& book = page.progress.spells();
    // 제목 자리 = 페이지 폭 − 칸 네 개 (처음 60픽셀은 굵은 "마술 칸 3/4"가 잘렸어요)
    const float title_w = page.box.w - (kSlotCell + kSlotGap) * static_cast<float>(progression::SpellBook::kMaxSlots);
    ui_line(page.painter, {page.box.x, y + (kSlotCell - line_h) * 0.5f, title_w, line_h},
            std::format("마술 칸 {}/{}", open, progression::SpellBook::kMaxSlots), theme.text_focused, page.alpha,
            engine::TextAlign::Left, engine::FontStyle::Bold);
    const std::size_t selected = book.selected_slot(open);
    for (std::size_t i = 0; i < progression::SpellBook::kMaxSlots; ++i) {
        const float x = page.box.x + page.box.w - (kSlotCell + kSlotGap) * static_cast<float>(progression::SpellBook::kMaxSlots - i) + kSlotGap;
        const engine::RectF cell{x, y, kSlotCell, kSlotCell};
        const bool locked = i >= open;  // 지능이 모자라 아직 닫힌 칸
        const float alpha = page.alpha * (locked ? kMenuDisabledAlpha : 1.0f);
        renderer.fill_rect(cell, engine::with_alpha(kMenuCellFill, alpha));
        const auto id = locked ? std::string_view{} : book.in_slot(i, open);
        const auto* spell = id.empty() ? nullptr : page.items.spells.find(id);
        if (spell != nullptr) ui_icon(renderer, page.spell_icons, spell->icon, {cell.x + 3.0f, cell.y + 3.0f}, alpha);
        const bool chosen = !locked && spell != nullptr && i == selected;
        ui_border(renderer, cell, engine::with_alpha(chosen ? kSelectedSlot : kMenuCellBorder, alpha));
    }
    return y + kSlotCell + 6.0f;
}

// 3) 고른 마술 설명
void draw_details(const MenuPage& page, float y, const combat::Spell& spell) {
    const auto& theme = page.painter.theme();
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    const float w = page.box.w;
    const float x = page.box.x;
    const int intelligence = page.progress.stats()[progression::StatId::Intelligence];
    const bool short_int = intelligence < spell.intelligence_required;
    ui_line(page.painter, {x, y, w, line_h}, std::format("요구 지능 {} (지금 {})", spell.intelligence_required, intelligence),
            short_int ? kMenuWarn : theme.text, page.alpha);
    const std::string mp = spell.mp_per_second > 0.0f ? std::format("MP 초당 {:g}", spell.mp_per_second)
                                                       : std::format("MP {}", spell.mp_cost);
    ui_line(page.painter, {x, y + line_h, w, line_h}, std::format("{} · 시전 {:g}초", mp, spell.cast_seconds), theme.text,
            page.alpha);
    const std::string power = spell.motion == combat::SpellMotion::Beam ? std::format("위력 초당 {:g}", spell.power)
                                                                        : std::format("위력 {:g}", spell.power);
    ui_line(page.painter, {x, y + line_h * 2.0f, w, line_h}, std::format("{} · {}", power, motion_name(spell.motion)),
            theme.text, page.alpha);
}

}  // namespace

void draw_spell_page(const MenuPage& page, std::size_t focused_row) {
    auto& renderer = page.painter.renderer();
    const auto& theme = page.painter.theme();
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    const auto& book = page.progress.spells();
    const std::size_t open = std::min<std::size_t>(static_cast<std::size_t>(page.progress.derived().spell_slots),
                                                   progression::SpellBook::kMaxSlots);
    float y = draw_slots(page, page.box.y, open);
    // 2) 배운 마술 목록
    const auto& learned = book.learned();
    const combat::Spell* focused = nullptr;
    for (std::size_t i = 0; i < learned.size() && i < kListRows; ++i) {
        const auto* spell = page.items.spells.find(learned[i]);
        const bool is_focused = i == focused_row;
        if (is_focused) focused = spell;
        const engine::RectF row{page.box.x, y, page.box.w, kListRow};
        if (is_focused) renderer.fill_rect(row, engine::with_alpha(kMenuRowFocus, page.alpha));
        if (spell != nullptr) ui_icon(renderer, page.spell_icons, spell->icon, {row.x + 1.0f, row.y + 1.0f}, page.alpha);
        const float text_y = y + (kListRow - line_h) * 0.5f + 1.0f;
        ui_line(page.painter, {row.x + 22.0f, text_y, row.w - 60.0f, line_h}, spell != nullptr ? spell->name : learned[i],
                is_focused ? theme.text_focused : theme.text, page.alpha);
        if (const std::size_t slot = slot_of(book, learned[i], open); slot > 0) {
            ui_line(page.painter, {row.x, text_y, row.w - 2.0f, line_h}, std::format("칸 {}", slot), theme.text_dim,
                    page.alpha, engine::TextAlign::Right);
        }
        y += kListRow;
    }
    if (learned.empty()) ui_line(page.painter, {page.box.x, y, page.box.w, line_h}, "배운 마술이 없어요", theme.text_dim, page.alpha);
    y = page.box.y + kSlotCell + 6.0f + kListRow * static_cast<float>(kListRows) + 4.0f;  // 설명 자리는 늘 같은 높이
    // 3) 설명, 4) 칸 바꾸기 안내
    if (focused != nullptr) draw_details(page, y, *focused);
    ui_line(page.painter, {page.box.x, page.box.y + page.box.h - line_h, page.box.w, line_h},
            "마술 칸은 캣닢 쿠션에서 바꿔요", theme.text_dim, page.alpha);
}

}  // namespace wind::client
