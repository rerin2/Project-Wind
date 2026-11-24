// 이 파일은 무엇을 하나요?
// 쿠션 "마술 칸" 화면 안쪽: 제목, 칸 네 줄, 배운 마술 목록, 키 안내.
// 자리·색 값의 이유는 docs/VISUALS.md "Cushion spell slots" 절에 있어요.

#include "app/spell_slot_view.hpp"

#include "app/menu_page.hpp"
#include "app/ui_text.hpp"
#include "engine/render/renderer.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/combat/spell_shelf.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/ui/spell_slot_sheet.hpp"

#include "wind/progression/spell_book.hpp"

#include <cmath>
#include <format>
#include <string>

namespace wind::client {
namespace {

using game::SpellSlotSheet;

constexpr float kSlotRow = 22.0f;      // 칸 한 줄 높이
constexpr float kListRow = 18.0f;      // 마술 목록 한 줄 높이
constexpr float kSlotColumn = 132.0f;  // 왼쪽 칸 줄 너비 (오른쪽은 목록)
constexpr float kColumnGap = 10.0f;    // 두 줄 사이
constexpr float kLabelWidth = 30.0f;   // "칸 1" 자리 (26이면 아이콘에 붙어 보였어요)

// 마술 이름 (선반에 없으면 id)
std::string_view spell_name(const game::SpellShelf* shelf, std::string_view id) {
    const auto* spell = shelf != nullptr ? shelf->find(id) : nullptr;
    return spell != nullptr ? std::string_view{spell->name} : id;
}

// 마술 아이콘 (선반에 없으면 그리지 않아요)
void spell_icon(const SpellSlotDraw& d, std::string_view id, engine::Vec2 at, float alpha) {
    const auto* spell = d.shelf != nullptr ? d.shelf->find(id) : nullptr;
    if (spell != nullptr) ui_icon(d.painter.renderer(), d.icons, spell->icon, at, alpha);
}

// 왼쪽: 칸 네 줄
void draw_slots(const SpellSlotDraw& d, float y) {
    const auto& theme = d.painter.theme();
    const float line_h = d.painter.line_height(engine::FontStyle::Regular);
    const auto& book = d.progress.spells();
    const std::size_t open = SpellSlotSheet::open_slots(d.progress);
    const bool picking = d.sheet.phase() == SpellSlotSheet::Phase::Picking;
    for (std::size_t i = 0; i < progression::SpellBook::kMaxSlots; ++i) {
        const bool locked = i >= open;
        const bool focused = i == d.sheet.slot();
        const float alpha = locked ? kMenuDisabledAlpha : 1.0f;
        const engine::RectF row{d.content.x, y, kSlotColumn, kSlotRow - 2.0f};
        // 고른 칸: 1단계면 금빛 바탕, 2단계면 고르는 중인 칸이라 테두리만
        if (focused && !picking) d.painter.renderer().fill_rect(row, kMenuRowFocus);
        if (focused && picking) ui_border(d.painter.renderer(), row, engine::with_alpha(theme.text_focused, 0.7f));
        const float text_y = y + (kSlotRow - 2.0f - line_h) * 0.5f + 1.0f;
        ui_line(d.painter, {row.x + 3.0f, text_y, kLabelWidth, line_h}, std::format("칸 {}", i + 1),
                focused ? theme.text_focused : theme.text_dim, alpha);
        const float name_x = row.x + 3.0f + kLabelWidth + 20.0f;
        const float name_w = row.x + row.w - name_x;
        if (locked) {
            ui_line(d.painter, {name_x, text_y, name_w, line_h}, "잠김", theme.text_dim, alpha);
        } else if (const auto id = book.in_slot(i, open); !id.empty()) {
            spell_icon(d, id, {row.x + 3.0f + kLabelWidth, y + 1.0f}, alpha);
            ui_line(d.painter, {name_x, text_y, name_w, line_h}, spell_name(d.shelf, id), theme.text, alpha);
        } else {
            ui_line(d.painter, {name_x, text_y, name_w, line_h}, "비어 있음", theme.text_dim, alpha);
        }
        y += kSlotRow;
    }
}

// 오른쪽: 배운 마술 목록 + "비우기" (1단계에서는 흐리게 미리 보여 줘요)
void draw_list(const SpellSlotDraw& d, float y) {
    const auto& theme = d.painter.theme();
    const float line_h = d.painter.line_height(engine::FontStyle::Regular);
    const auto& book = d.progress.spells();
    const std::size_t open = SpellSlotSheet::open_slots(d.progress);
    const int intelligence = d.progress.stats()[progression::StatId::Intelligence];
    const float x = std::round(d.content.x + kSlotColumn + kColumnGap + d.shake);  // 거절되면 목록이 흔들려요
    const float w = d.content.x + d.content.w - (d.content.x + kSlotColumn + kColumnGap);
    const auto& learned = book.learned();
    const bool active = d.sheet.phase() == SpellSlotSheet::Phase::Picking;  // 2단계에서만 고를 수 있어요
    const float dim = active ? 1.0f : 0.5f;                                 // 1단계에서는 흐리게
    for (std::size_t i = 0; i <= learned.size(); ++i) {
        const bool focused = active && i == d.sheet.pick();
        const engine::RectF row{x, y, w, kListRow - 1.0f};
        if (focused) d.painter.renderer().fill_rect(row, kMenuRowFocus);
        const float text_y = y + (kListRow - 1.0f - line_h) * 0.5f + 1.0f;
        if (i == learned.size()) {  // 마지막 줄: 비우기
            ui_line(d.painter, {x + 20.0f, text_y, w - 20.0f, line_h}, "비우기", focused ? theme.text_focused : theme.text_dim,
                    dim);
            break;
        }
        const std::string& id = learned[i];
        const auto* spell = d.shelf != nullptr ? d.shelf->find(id) : nullptr;
        const bool short_int = spell == nullptr || intelligence < spell->intelligence_required;
        spell_icon(d, id, {x + 1.0f, y}, dim * (short_int ? kMenuDisabledAlpha : 1.0f));
        ui_line(d.painter, {x + 20.0f, text_y, w - 56.0f, line_h}, spell_name(d.shelf, id),
                focused ? theme.text_focused : theme.text, dim * (short_int ? 0.7f : 1.0f));
        // 오른쪽 끝: 모자라면 요구 지능(빨강), 다른 칸에 있으면 "칸 N"
        std::string tag;
        for (std::size_t s = 0; s < open; ++s) {
            if (book.in_slot(s, open) == id) tag = std::format("칸 {}", s + 1);
        }
        if (short_int && spell != nullptr) tag = std::format("지능 {}", spell->intelligence_required);
        ui_line(d.painter, {x, text_y, w - 2.0f, line_h}, tag, short_int ? kMenuWarn : theme.text_dim, dim,
                engine::TextAlign::Right);
        y += kListRow;
    }
}

}  // namespace

void draw_spell_slot_sheet(const SpellSlotDraw& d) {
    const auto& theme = d.painter.theme();
    const float line_h = d.painter.line_height(engine::FontStyle::Regular);
    float y = d.content.y;
    // 1) 제목과 지금 지능
    ui_line(d.painter, {d.content.x, y, d.content.w, line_h}, "마술 칸", theme.text_focused, 1.0f, engine::TextAlign::Left,
            engine::FontStyle::Bold);
    ui_line(d.painter, {d.content.x, y, d.content.w, line_h},
            std::format("지능 {}", d.progress.stats()[progression::StatId::Intelligence]), theme.text_dim, 1.0f,
            engine::TextAlign::Right);
    y += line_h + 8.0f;
    // 2) 칸, 3) 목록
    draw_slots(d, y);
    draw_list(d, y);
    // 4) 키 안내
    const bool picking = d.sheet.phase() == SpellSlotSheet::Phase::Picking;
    ui_line(d.painter, {d.content.x, d.content.y + d.content.h - line_h, d.content.w, line_h},
            picking ? "Enter: 넣기   Esc: 칸 고르기" : "Enter: 마술 고르기   Esc: 돌아가기", theme.text_dim, 1.0f,
            engine::TextAlign::Center);
}

}  // namespace wind::client
