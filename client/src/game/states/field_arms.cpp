// 이 파일은 무엇을 하나요?
// 필드의 무기 담당: 끼고 있는 무기를 주인공 손에 쥐여 주고, 인벤토리 "장착" 요청을 처리해요.

#include "game/states/field_arms.hpp"

#include "game/entities/player_motion.hpp"
#include "game/items/weapon_rack.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/combat/heavy_weapon.hpp"
#include "wind/items/equipment.hpp"
#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/log.hpp"

namespace wind::game {

HeldWeapon FieldArms::held() const {
    const std::string& id = gear_.worn.worn(items::EquipSlot::Weapon);
    const combat::Weapon* weapon = id.empty() ? nullptr : gear_.weapons.find(id);
    if (weapon == nullptr) {
        if (!id.empty()) log::warn("Worn weapon {} is not on the weapon rack; bare hands", id);
        return {};  // 맨손: 콤보가 비어 공격이 안 나가요
    }
    const bool heavy = progress_.derived(weapon->weight).heavy;  // 한 곳의 계산 함수 (명세 8.2, 10.3)
    return {combat::wield(*weapon, heavy), heavy};
}

void FieldArms::arm(PlayerMotion& hero) const {
    hero.set_weapon(held().weapon);
}

void FieldArms::step(PlayerMotion& hero) {
    for (const std::size_t slot : requests_) equip(slot, hero);
    requests_.clear();
}

// 장착: 먼저 바꿀 수 있는지 보고, 될 때만 바꿔 끼워요 (전부 성공 또는 전부 실패)
void FieldArms::equip(std::size_t bag_slot, PlayerMotion& hero) {
    const auto& stack = items_.bag.at(bag_slot);
    if (!stack) return;  // 그사이 칸이 비었어요
    const items::ItemStack item{stack->item_id, 1};
    if (!hero.can_change_weapon()) {  // 공격·구르기·맞는 중 등
        notices_.push_back({item, LootNotice::Kind::CannotEquip});
        return;
    }
    if (gear_.worn.equip_from(items_.bag, bag_slot, items_.catalog) != items::Equipment::Result::Done) {
        return;  // 장비가 아니에요 (화면이 이미 막아요). 아무것도 안 바뀌었어요
    }
    const HeldWeapon held_now = held();
    hero.set_weapon(held_now.weapon);
    notices_.push_back({item, held_now.heavy ? LootNotice::Kind::EquippedHeavy : LootNotice::Kind::Equipped});
    log::info("Equipped {}{}", item.item_id, held_now.heavy ? " (heavy)" : "");
}

}  // namespace wind::game
