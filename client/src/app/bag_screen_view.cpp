// 이 파일은 무엇을 하나요?
// 인벤토리 화면 그리기. 자리·색 값의 이유는 docs/VISUALS.md "Bag screen" 절에 있어요.

#include "app/bag_screen_view.hpp"

#include "engine/render/renderer.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/presentation/loot_view.hpp"
#include "game/presentation/rarity_color.hpp"
#include "game/items/weapon_rack.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/ui/bag_screen.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <string>
#include <string_view>

namespace wind::client {
namespace {

using game::BagScreen;

constexpr float kCell = 26.0f;            // 칸 크기 (아이콘 16 + 여백 5씩, 개수 글자 자리)
constexpr float kPitch = 28.0f;           // 칸 간격 (칸 26 + 틈 2)
constexpr float kDetailLines = 5.0f;      // 설명 칸 높이 (줄 수: 이름, 종류, 설명 3줄)
constexpr float kMenuWidth = 72.0f;       // 작은 메뉴 너비
constexpr float kMenuButton = 18.0f;      // 작은 메뉴 버튼 높이
constexpr float kDisabledAlpha = 0.4f;    // 못 고르는 항목
constexpr engine::Color kCellFill{0, 0, 0, 90};           // 칸 바탕 (살짝 어둡게)
constexpr engine::Color kCellFocusFill{255, 233, 168, 46}; // 고른 칸 바탕 (옅은 금빛)
constexpr engine::Color kEmptyBorder{90, 96, 112, 160};   // 빈 칸 테두리 (흐린 회색)
constexpr engine::Color kEffectColor{142, 220, 128, 255}; // 효과 글자 (연두, 레벨업 화면과 같아요)
constexpr engine::Color kCountBack{0, 0, 0, 150};         // 개수 글자 뒤 작은 띠 (아이콘 위에서도 읽히게)

constexpr engine::Color kHeavyColor{236, 96, 96, 255};    // 무거움 글자 (빨강, 15a)

constexpr std::array<std::string_view, BagScreen::kActionCount> kActionLabels{"사용", "버리기", "취소"};
constexpr std::string_view kEquipLabel = "장착";  // 장비 칸에서는 "사용" 자리에 (15a)

// 이 칸이 장비인가요? (작은 메뉴의 첫 항목을 "장착"이라고 보여 줘요)
bool is_equipment(const items::Inventory& bag, const items::ItemCatalog& catalog, std::size_t slot) {
    const auto& stack = bag.at(slot);
    const auto* def = stack ? catalog.find(stack->item_id) : nullptr;
    return def != nullptr && def->equip.has_value();
}

// 글자 한 줄 (자르기, 그림자 있음)
void line(engine::UiPainter& painter, engine::RectF box, std::string_view text, engine::Color color, float alpha,
          engine::TextAlign align = engine::TextAlign::Left, engine::FontStyle font = engine::FontStyle::Regular) {
    engine::TextStyle style;
    style.font = font;
    style.color = color;
    style.align = align;
    style.max_lines = 1;
    style.wrap = false;
    painter.text(box, text, style, alpha);
}

// 1픽셀 테두리 (네 변을 색칠한 네모로)
void border(engine::Renderer& renderer, engine::RectF r, engine::Color color, float width = 1.0f) {
    renderer.fill_rect({r.x, r.y, r.w, width}, color);
    renderer.fill_rect({r.x, r.y + r.h - width, r.w, width}, color);
    renderer.fill_rect({r.x, r.y, width, r.h}, color);
    renderer.fill_rect({r.x + r.w - width, r.y, width, r.h}, color);
}

// 종류와 효과 한 줄: "소비 · HP +50", "재료", "무기"
std::string kind_text(const items::ItemDef& def) {
    switch (def.kind) {
        case items::ItemKind::Consumable: {
            std::string text = "소비";
            if (def.use && def.use->heal_hp > 0) text += std::format(" · HP +{}", def.use->heal_hp);
            if (def.use && def.use->heal_mp > 0) text += std::format(" · MP +{}", def.use->heal_mp);
            return text;
        }
        case items::ItemKind::Material: return "재료";
        case items::ItemKind::Weapon: return "무기";
    }
    return {};
}

// 무기 줄 (15a): "무기 · 무게 6", 들 수 있는 무게보다 무거우면 "무기 · 무게 14 / 8 · 무거움"
std::string weapon_text(const combat::Weapon& weapon, float carry_weight, bool heavy) {
    if (!heavy) return std::format("무기 · 무게 {:g}", weapon.weight);
    return std::format("무기 · 무게 {:g} / {:g} · 무거움", weapon.weight, std::floor(carry_weight));
}

// 칸 수에 맞는 줄 수 (24칸 → 4줄)
std::size_t row_count(std::size_t slots) {
    return (slots + BagScreen::kColumns - 1) / BagScreen::kColumns;
}

}  // namespace

engine::Vec2 BagScreenView::size(const engine::UiPainter& painter, const items::Inventory& bag) {
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const float grid_w = kPitch * static_cast<float>(BagScreen::kColumns) - (kPitch - kCell);
    const float grid_h = kPitch * static_cast<float>(row_count(bag.slot_count())) - (kPitch - kCell);
    return {grid_w, line_h + 6.0f + grid_h + 8.0f + line_h * kDetailLines};  // 칸 수 줄 + 칸 + 설명
}

void BagScreenView::draw(engine::UiPainter& painter, engine::Vec2 origin, float alpha, bool show_menu,
                         const game::BagScreen& bag_screen, const items::Inventory& bag,
                         const items::ItemCatalog& catalog) const {
    const auto& theme = painter.theme();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const float grid_w = size(painter, bag).x;
    const float grid_h = kPitch * static_cast<float>(row_count(bag.slot_count())) - (kPitch - kCell);
    const float x = origin.x;
    float y = origin.y;
    // 1) 제목과 쓴 칸 수
    line(painter, {x, y, grid_w, line_h}, "가방", theme.text_focused, alpha, engine::TextAlign::Left, engine::FontStyle::Bold);
    const auto used = bag.slot_count() - bag.free_slots();
    line(painter, {x, y, grid_w, line_h}, std::format("{}/{}", used, bag.slot_count()), theme.text_dim, alpha,
         engine::TextAlign::Right);
    y += line_h + 6.0f;
    // 2) 칸들 (고른 칸의 네모는 작은 메뉴 자리로도 써요)
    engine::RectF focused_cell{};
    for (std::size_t slot = 0; slot < bag.slot_count(); ++slot) {
        const float cx = x + kPitch * static_cast<float>(slot % BagScreen::kColumns);
        const float cy = y + kPitch * static_cast<float>(slot / BagScreen::kColumns);
        const engine::RectF cell{cx, cy, kCell, kCell};
        const bool focused = slot == bag_screen.cursor();
        if (focused) focused_cell = cell;
        draw_slot(painter, cell, slot, focused, bag, catalog, alpha);
    }
    y += grid_h + 8.0f;
    // 3) 고른 아이템 설명
    draw_details(painter, {x, y, grid_w, line_h * kDetailLines}, bag_screen.cursor(), bag, catalog, alpha);
    // 4) 작은 메뉴 (맨 위에, 다 나타난 뒤에만). 키 안내는 메뉴 창 맨 아래에 있어요 (15e)
    if (bag_screen.menu_open() && show_menu) draw_menu(painter, focused_cell, bag_screen, bag, catalog);
}

void BagScreenView::draw_slot(engine::UiPainter& painter, engine::RectF cell, std::size_t slot, bool focused,
                              const items::Inventory& bag, const items::ItemCatalog& catalog, float alpha) const {
    auto& renderer = painter.renderer();
    renderer.fill_rect(cell, engine::with_alpha(focused ? kCellFocusFill : kCellFill, alpha));
    const auto& stack = bag.at(slot);
    const auto* def = stack ? catalog.find(stack->item_id) : nullptr;
    // 테두리: 빈 칸은 흐린 회색, 아이템은 희귀도 색, 고른 칸은 금빛 두 줄
    const engine::Color edge = stack ? game::rarity_color(def != nullptr ? def->rarity : items::Rarity::Common)
                                     : kEmptyBorder;
    border(renderer, cell, engine::with_alpha(edge, alpha * (stack ? 0.8f : 1.0f)));
    if (focused) border(renderer, {cell.x - 1.0f, cell.y - 1.0f, cell.w + 2.0f, cell.h + 2.0f},
                        engine::with_alpha(painter.theme().text_focused, alpha), 1.0f);
    if (!stack) return;
    // 아이콘: 칸 가운데
    constexpr int kIcon = game::LootView::kIconSize;
    engine::SpriteDraw sprite;
    sprite.texture = icons_;
    sprite.source = {(def != nullptr ? def->icon : 0) * kIcon, 0, kIcon, kIcon};
    sprite.dest = {cell.x + 5.0f, cell.y + 4.0f, static_cast<float>(kIcon), static_cast<float>(kIcon)};
    sprite.tint = engine::with_alpha({255, 255, 255, 255}, alpha);
    renderer.draw_sprite(sprite);
    // 개수: 2개 이상이면 오른쪽 아래 구석에, 작은 어두운 띠 위에 (아이콘을 덜 가리게)
    if (stack->count > 1) {
        const float line_h = painter.line_height(engine::FontStyle::Regular);
        const std::string count = std::to_string(stack->count);
        const float text_w = painter.measure(engine::FontStyle::Regular, count);
        const float text_y = cell.y + cell.h - line_h + 2.0f;  // 칸 바닥에 붙여요
        renderer.fill_rect({cell.x + cell.w - text_w - 2.0f, text_y + 2.0f, text_w + 1.0f, line_h - 3.0f},
                           engine::with_alpha(kCountBack, alpha));
        line(painter, {cell.x, text_y, cell.w - 1.0f, line_h}, count, painter.theme().text, alpha, engine::TextAlign::Right);
    }
}

void BagScreenView::draw_details(engine::UiPainter& painter, engine::RectF box, std::size_t slot,
                                 const items::Inventory& bag, const items::ItemCatalog& catalog, float alpha) const {
    const auto& theme = painter.theme();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const auto& stack = bag.at(slot);
    if (!stack) {
        line(painter, {box.x, box.y, box.w, line_h}, "빈 칸", theme.text_dim, alpha);
        return;
    }
    const auto* def = catalog.find(stack->item_id);
    if (def == nullptr) {  // 도감에 없는 아이템 (옛 저장 파일 등): id라도 보여 줘요
        line(painter, {box.x, box.y, box.w, line_h}, std::format("{} ×{}", stack->item_id, stack->count), theme.text, alpha);
        return;
    }
    // 이름 (희귀도 색) + 개수
    line(painter, {box.x, box.y, box.w, line_h}, def->name, game::rarity_color(def->rarity), alpha,
         engine::TextAlign::Left, engine::FontStyle::Bold);
    line(painter, {box.x, box.y, box.w, line_h}, std::format("×{}", stack->count), theme.text, alpha,
         engine::TextAlign::Right);
    // 종류와 효과 (무기면 무게, 무거우면 빨갛게, 15a)
    const combat::Weapon* weapon = weapons_ != nullptr ? weapons_->find(def->id) : nullptr;
    if (weapon != nullptr && progress_ != nullptr) {
        const auto load = progress_->derived(weapon->weight);  // 한 곳의 계산 함수 (명세 8.2, 10.3)
        line(painter, {box.x, box.y + line_h, box.w, line_h}, weapon_text(*weapon, load.carry_weight, load.heavy),
             load.heavy ? kHeavyColor : theme.text_dim, alpha);
    } else {
        line(painter, {box.x, box.y + line_h, box.w, line_h}, kind_text(*def),
             def->use ? kEffectColor : theme.text_dim, alpha);
    }
    // 설명 (줄바꿈, 3줄까지)
    engine::TextStyle style;
    style.color = theme.text;
    style.max_lines = 3;
    painter.text({box.x, box.y + line_h * 2.0f + 2.0f, box.w, line_h * 3.0f}, def->description, style, alpha);
}

void BagScreenView::draw_menu(engine::UiPainter& painter, engine::RectF cell, const game::BagScreen& bag_screen,
                              const items::Inventory& bag, const items::ItemCatalog& catalog) const {
    const float height = kMenuButton * static_cast<float>(BagScreen::kActionCount) + 8.0f;
    const engine::RectF rect{cell.x + cell.w + 4.0f, cell.y, kMenuWidth, height};  // 고른 칸 바로 오른쪽
    painter.panel(rect);
    for (std::size_t i = 0; i < BagScreen::kActionCount; ++i) {
        const auto action = static_cast<BagScreen::Action>(i);
        const bool enabled = bag_screen.action_enabled(action, bag, catalog);
        const engine::RectF button{rect.x + 4.0f, rect.y + 4.0f + kMenuButton * static_cast<float>(i), rect.w - 8.0f,
                                   kMenuButton};
        const bool equip = action == BagScreen::Action::Use && is_equipment(bag, catalog, bag_screen.cursor());
        painter.button(button, equip ? kEquipLabel : kActionLabels[i], enabled && bag_screen.menu_choice() == action,
                       enabled ? 1.0f : kDisabledAlpha);
    }
}

}  // namespace wind::client
