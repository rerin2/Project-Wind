// 이 파일은 무엇을 하나요?
// 아이템 HUD: 필드의 줍기 알림 가져오기, 무기·마술 칸에 보여 줄 것 모으기, 그리기.
// (인벤토리 화면은 15e부터 일시정지 메뉴의 가방 탭이에요: PauseMenuPanel)

#include "app/item_hud.hpp"

#include "app/item_setup.hpp"
#include "app/loot_toast_drawing.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/states/field_state.hpp"

#include "wind/items/equipment.hpp"

#include <algorithm>

namespace wind::client {

ItemHud::ItemHud(ItemSetup& items, const game::HeroProgress& progress, engine::TextureHandle icons,
                 engine::TextureHandle spell_icons)
    : items_(items), progress_(progress), icons_(icons), hands_(icons, spell_icons), quick_(icons) {}

void ItemHud::update(float dt_seconds, game::FieldState& field) {
    for (const auto& notice : field.take_loot_notices()) toasts_.push(notice, items_.catalog);  // 주움·가득 참 (14b)
    toasts_.update(dt_seconds);
    hands_.update(dt_seconds, hands_info(field));  // 무기·마술 칸 (15b)
    quick_.update(dt_seconds, quick_info());        // 단축 칸 (15f)
}

std::array<QuickSlotInfo, QuickSlotsHud::kCount> ItemHud::quick_info() const {
    std::array<QuickSlotInfo, QuickSlotsHud::kCount> info{};
    for (std::size_t i = 0; i < info.size(); ++i) {
        const std::string& id = items_.quick.at(i);
        if (id.empty()) continue;  // 빈 칸
        const auto* def = items_.catalog.find(id);
        info[i].assigned = true;
        info[i].icon = def != nullptr ? def->icon : 0;
        info[i].count = items_.bag.count_of(id);  // 0개여도 등록은 남아요 (개발자와 정함)
    }
    return info;
}

void ItemHud::draw(engine::UiPainter& painter, engine::Vec2 screen) const {
    hands_.draw(painter, screen);                                                   // 맨 아래: 무기·마술 칸 (15b)
    draw_loot_toasts(painter, icons_, screen, toasts_, HandsHud::kReservedHeight);  // 그 위: 알림 줄
    quick_.draw(painter, screen);                                                   // 아래 가운데: 단축 칸 (15f)
}

// 무기 칸: 끼고 있는 무기의 아이콘. 마술 칸: 필드가 쥐여 준 마술, MP, 공책의 칸들 (15b)
HandsInfo ItemHud::hands_info(const game::FieldState& field) const {
    HandsInfo info;
    const auto* weapon = items_.catalog.find(items_.worn.worn(items::EquipSlot::Weapon));
    if (weapon != nullptr) info.weapon_icon = weapon->icon;
    if (const combat::Spell* spell = field.spells().selected()) {
        info.spell_icon = spell->icon;
        info.spell_name = spell->name;
        // 시작할 MP (빛 탄·근접은 한 번 쓸 MP, 광선은 한 번 맞히는 동안의 MP)가 있어야 흐리지 않아요
        info.spell_affordable = field.mana().current() >= combat::start_mp(*spell);  // 시전기와 같은 기준 (15d)
    }
    const auto& book = progress_.spells();
    info.open_slots = std::min<std::size_t>(static_cast<std::size_t>(progress_.derived().spell_slots),
                                            progression::SpellBook::kMaxSlots);
    for (std::size_t i = 0; i < info.open_slots; ++i) {
        if (!book.in_slot(i, info.open_slots).empty()) info.filled_mask |= 1u << i;
    }
    info.selected_slot = book.selected_slot(info.open_slots);
    info.switch_count = field.spells().switch_count();
    return info;
}

}  // namespace wind::client
