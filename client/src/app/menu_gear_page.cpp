// 이 파일은 무엇을 하나요?
// "장비" 탭: 장비 무게, 칸 네 줄, 고른 칸 설명을 그려요. 자리 값의 이유는 docs/VISUALS.md "Pause menu" 절.

#include "app/menu_gear_page.hpp"

#include "app/item_setup.hpp"
#include "app/menu_page.hpp"
#include "app/ui_text.hpp"
#include "engine/render/renderer.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/presentation/rarity_color.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/combat/damage.hpp"
#include "wind/items/equip_slot.hpp"

#include <array>
#include <cmath>
#include <format>
#include <string>

namespace wind::client {
namespace {

constexpr float kRowHeight = 24.0f;  // 칸 한 줄 높이 (칸 20 + 틈 4)
constexpr float kCell = 20.0f;       // 아이콘 칸 크기
constexpr float kSlotNameWidth = 40.0f;  // "장신구" 자리
constexpr std::array<std::string_view, items::kEquipSlotCount> kSlotNames{"무기", "머리", "몸", "장신구"};

// 무기 설명 줄들 (공격력은 지금 힘으로 계산한 값, 명세 8.2)
void draw_weapon_details(const MenuPage& page, engine::RectF box, const combat::Weapon& weapon,
                         const progression::DerivedStats& load) {
    const auto& theme = page.painter.theme();
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    const float power = combat::physical_attack_power(weapon.base_power, weapon.strength_scaling, load.strength_bonus);
    const std::array<std::string, 3> rows{
        std::format("공격력 {:.0f} (기본 {:g} · 힘 보정 {:g})", std::floor(power), weapon.base_power, weapon.strength_scaling),
        std::format("치명타 +{:.0f}% · 콤보 {}타", weapon.crit_bonus * 100.0f, weapon.combo.size()),
        std::format("한 번 휘두를 때 스태미나 {:g}", weapon.stamina_per_attack),
    };
    for (std::size_t i = 0; i < rows.size(); ++i) {
        ui_line(page.painter, {box.x, box.y + line_h * static_cast<float>(i), box.w, line_h}, rows[i], theme.text,
                page.alpha);
    }
}

}  // namespace

void draw_gear_page(const MenuPage& page, std::size_t focused_row) {
    auto& painter = page.painter;
    auto& renderer = painter.renderer();
    const auto& theme = painter.theme();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const engine::RectF& box = page.box;
    const auto& worn_weapon = page.items.worn.worn(items::EquipSlot::Weapon);
    const combat::Weapon* weapon = page.items.weapons.find(worn_weapon);
    const auto load = page.progress.derived(weapon != nullptr ? weapon->weight : 0.0f);  // 무거움은 한 곳의 계산 함수로
    float y = box.y;
    // 1) 장비 무게 / 들 수 있는 무게
    ui_line(painter, {box.x, y, box.w, line_h}, "장비 무게", theme.text_focused, page.alpha, engine::TextAlign::Left,
            engine::FontStyle::Bold);
    const std::string weight = std::format("{:g} / {:g}{}", load.equipped_weight, std::floor(load.carry_weight),
                                           load.heavy ? " · 무거움" : "");
    ui_line(painter, {box.x, y, box.w, line_h}, weight, load.heavy ? kMenuWarn : theme.text, page.alpha,
            engine::TextAlign::Right);
    y += line_h + 6.0f;
    // 2) 칸 네 줄
    for (std::size_t i = 0; i < items::kEquipSlotCount; ++i) {
        const auto slot = static_cast<items::EquipSlot>(i);
        const bool focused = i == focused_row;
        const engine::RectF row{box.x, y, box.w, kRowHeight - 2.0f};
        if (focused) renderer.fill_rect(row, engine::with_alpha(kMenuRowFocus, page.alpha));
        const engine::RectF cell{box.x + 1.0f, y + 1.0f, kCell, kCell};
        renderer.fill_rect(cell, engine::with_alpha(kMenuCellFill, page.alpha));
        const auto* def = page.items.catalog.find(page.items.worn.worn(slot));
        ui_border(renderer, cell, engine::with_alpha(def != nullptr ? game::rarity_color(def->rarity) : kMenuCellBorder,
                                                     page.alpha));
        if (def != nullptr) ui_icon(renderer, page.item_icons, def->icon, {cell.x + 2.0f, cell.y + 2.0f}, page.alpha);
        const float text_y = y + (kCell - line_h) * 0.5f + 1.0f;  // 칸 가운데 높이
        const float name_x = cell.x + kCell + 6.0f;
        ui_line(painter, {name_x, text_y, kSlotNameWidth, line_h}, kSlotNames[i],
                focused ? theme.text_focused : theme.text_dim, page.alpha);
        const float item_x = name_x + kSlotNameWidth;
        if (def != nullptr) {
            ui_line(painter, {item_x, text_y, box.x + box.w - item_x, line_h}, def->name,
                    game::rarity_color(def->rarity), page.alpha);
        } else {
            ui_line(painter, {item_x, text_y, box.x + box.w - item_x, line_h}, "비어 있음", theme.text_dim, page.alpha);
        }
        y += kRowHeight;
    }
    y += 4.0f;
    // 3) 고른 칸 설명
    const engine::RectF details{box.x, y, box.w, box.y + box.h - y};
    if (focused_row == 0 && weapon != nullptr) {
        draw_weapon_details(page, details, *weapon, load);
        const auto* def = page.items.catalog.find(worn_weapon);
        if (def != nullptr) {  // 아이템 설명 (두 줄까지)
            engine::TextStyle style;
            style.color = theme.text_dim;
            style.max_lines = 2;
            painter.text({details.x, details.y + line_h * 3.0f + 4.0f, details.w, line_h * 2.0f}, def->description,
                         style, page.alpha);
        }
    } else if (focused_row == 0) {
        ui_line(painter, {details.x, details.y, details.w, line_h}, "맨손이에요. 가방에서 무기를 장착해요", theme.text_dim,
                page.alpha);
    } else {
        ui_line(painter, {details.x, details.y, details.w, line_h}, "이 칸의 장비는 아직 없어요", theme.text_dim,
                page.alpha);
    }
}

}  // namespace wind::client
