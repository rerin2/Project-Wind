// 이 파일은 무엇을 하나요?
// "능력치" 탭: 레벨·룬, 능력치, 그로부터 계산된 값들을 두 칸(이름 · 값)으로 그려요.

#include "app/menu_stats_page.hpp"

#include "app/item_setup.hpp"
#include "app/menu_page.hpp"
#include "app/ui_text.hpp"
#include "engine/render/renderer.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/combat/damage.hpp"
#include "wind/items/equip_slot.hpp"

#include <array>
#include <cmath>
#include <format>
#include <string>
#include <utility>

namespace wind::client {
namespace {

constexpr float kSectionGap = 6.0f;                          // 묶음 사이 틈
constexpr engine::Color kDivider{90, 96, 112, 120};          // 묶음 사이 가는 줄

// 이름은 왼쪽, 값은 오른쪽 (한 줄)
void pair_line(const MenuPage& page, float y, std::string_view name, const std::string& value, engine::Color color) {
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    ui_line(page.painter, {page.box.x, y, page.box.w, line_h}, name, page.painter.theme().text_dim, page.alpha);
    ui_line(page.painter, {page.box.x, y, page.box.w, line_h}, value, color, page.alpha, engine::TextAlign::Right);
}

// 묶음 사이 가는 줄
float divider(const MenuPage& page, float y) {
    page.painter.renderer().fill_rect({page.box.x, y + kSectionGap * 0.5f, page.box.w, 1.0f},
                                      engine::with_alpha(kDivider, page.alpha));
    return y + kSectionGap;
}

}  // namespace

void draw_stats_page(const MenuPage& page) {
    const auto& theme = page.painter.theme();
    const float line_h = page.painter.line_height(engine::FontStyle::Regular);
    const auto& progress = page.progress;
    float y = page.box.y;
    // 1) 레벨과 룬
    ui_line(page.painter, {page.box.x, y, page.box.w, line_h}, std::format("레벨 {}", progress.level()), theme.text_focused,
            page.alpha, engine::TextAlign::Left, engine::FontStyle::Bold);
    ui_line(page.painter, {page.box.x, y, page.box.w, line_h}, std::format("룬 {}", progress.runes()), theme.text,
            page.alpha, engine::TextAlign::Right);
    y = divider(page, y + line_h);
    // 2) 능력치 다섯 줄
    for (std::size_t i = 0; i < progression::kStatCount; ++i) {
        pair_line(page, y, kStatNames[i], std::to_string(progress.stats().values[i]), theme.text);
        y += line_h;
    }
    y = divider(page, y);
    // 3) 계산된 값들 (무기는 지금 끼고 있는 것으로, 무거움은 한 곳의 계산 함수로)
    const combat::Weapon* weapon = page.items.weapons.find(page.items.worn.worn(items::EquipSlot::Weapon));
    const auto derived = progress.derived(weapon != nullptr ? weapon->weight : 0.0f);
    const float physical = weapon != nullptr ? combat::physical_attack_power(weapon->base_power, weapon->strength_scaling,
                                                                            derived.strength_bonus)
                                             : 0.0f;
    const float magic = combat::magic_attack_power(combat::kBaseMagicPower, derived.intelligence_bonus);
    const std::array<std::pair<std::string_view, std::string>, 7> rows{{
        {"최대 HP", std::to_string(derived.max_hp)},
        {"최대 MP", std::to_string(derived.max_mp)},
        {"최대 스태미나", std::to_string(derived.max_stamina)},
        {"물리 공격력", std::format("{:.0f}", std::floor(physical))},
        {"마법 공격력", std::format("{:.0f}", std::floor(magic))},
        {"들 수 있는 무게", std::format("{:g}", std::floor(derived.carry_weight))},
        {"마술 칸", std::to_string(derived.spell_slots)},
    }};
    for (const auto& [name, value] : rows) {
        // 무게는 무거우면 빨갛게 (장비 탭과 같은 기준)
        const bool warn = derived.heavy && name == "들 수 있는 무게";
        pair_line(page, y, name, value, warn ? kMenuWarn : theme.text);
        y += line_h;
    }
}

}  // namespace wind::client
