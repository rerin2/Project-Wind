// 이 파일은 무엇을 하나요?
// 장비 끼기(바꿔 끼우기)와 벗기. 먼저 다 확인하고, 될 때만 바꿔요 (전부 성공 또는 전부 실패).

#include "wind/items/equipment.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <utility>

namespace wind::items {

Equipment::Result Equipment::equip_from(Inventory& bag, std::size_t bag_slot, const ItemCatalog& catalog) {
    const auto& stack = bag.at(bag_slot);
    if (!stack) return Result::EmptySlot;
    const ItemDef* def = catalog.find(stack->item_id);
    if (def == nullptr || !def->equip) return Result::NotEquipment;  // 장비만 낄 수 있어요
    auto taken = bag.take_at(bag_slot, stack->count);  // 장비는 max_stack 1이라 한 개예요
    if (!taken) return Result::EmptySlot;              // 위에서 확인했으니 일어나지 않아요
    std::string& slot = worn_[index(*def->equip)];
    if (!slot.empty()) {
        // 끼고 있던 것은 방금 비운 칸으로 (빈 칸이라 늘 들어가요)
        (void)bag.place_at(bag_slot, ItemStack{slot, 1});
    }
    slot = std::move(taken->item_id);
    return Result::Done;
}

Equipment::Result Equipment::unequip(EquipSlot slot, Inventory& bag, const ItemCatalog& catalog) {
    std::string& worn = worn_[index(slot)];
    if (worn.empty()) return Result::NothingWorn;
    if (!bag.try_add(catalog, worn, 1)) return Result::BagFull;  // 못 넣으면 그대로 끼고 있어요
    worn.clear();
    return Result::Done;
}

}  // namespace wind::items
