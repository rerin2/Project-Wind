// 이 파일은 무엇을 하나요?
// 장비 끼기·벗기 규칙(바꿔 끼우기, 전부 성공 또는 전부 실패)을 숙제 검사(테스트)해요. (명세 10.2, 10.3, 15a)
#include "wind/items/equipment.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::items::EquipSlot;
using wind::items::Equipment;
using wind::items::Inventory;
using wind::items::ItemCatalog;
using wind::items::ItemDef;
using wind::items::ItemStack;

// 검 두 자루(장비)와 물약(장비 아님)이 든 도감
ItemCatalog make_catalog() {
    ItemCatalog catalog;
    auto add = [&catalog](const char* id, std::int32_t max_stack, bool weapon) {
        ItemDef def;
        def.id = id;
        def.name = id;
        def.max_stack = max_stack;
        if (weapon) {
            def.kind = wind::items::ItemKind::Weapon;
            def.equip = EquipSlot::Weapon;
        }
        catalog.add(def);
    };
    add("longsword", 1, true);
    add("dagger", 1, true);
    add("potion", 20, false);
    return catalog;
}

TEST(Equipment, EquipFromBagMovesItemOutOfTheBag) {
    const auto catalog = make_catalog();
    Inventory bag;
    ASSERT_TRUE(bag.try_add(catalog, "dagger", 1));
    Equipment gear;
    EXPECT_EQ(gear.equip_from(bag, 0, catalog), Equipment::Result::Done);
    EXPECT_EQ(gear.worn(EquipSlot::Weapon), "dagger");
    EXPECT_FALSE(bag.at(0).has_value());  // 장비는 가방 칸을 차지하지 않아요
}

TEST(Equipment, SwappingPutsOldWeaponIntoTheSameSlot) {
    const auto catalog = make_catalog();
    Inventory bag;
    ASSERT_TRUE(bag.try_add(catalog, "potion", 3));
    ASSERT_TRUE(bag.try_add(catalog, "dagger", 1));  // 1번 칸
    Equipment gear;
    gear.wear(EquipSlot::Weapon, "longsword");
    EXPECT_EQ(gear.equip_from(bag, 1, catalog), Equipment::Result::Done);
    EXPECT_EQ(gear.worn(EquipSlot::Weapon), "dagger");
    ASSERT_TRUE(bag.at(1).has_value());
    EXPECT_EQ(bag.at(1)->item_id, "longsword");  // 벗은 검은 단검이 있던 칸으로
    EXPECT_EQ(bag.at(0)->count, 3);              // 다른 칸은 그대로
}

TEST(Equipment, SwappingWorksEvenWhenTheBagIsFull) {
    const auto catalog = make_catalog();
    Inventory bag(2);
    ASSERT_TRUE(bag.try_add(catalog, "potion", 1));
    ASSERT_TRUE(bag.try_add(catalog, "dagger", 1));
    ASSERT_EQ(bag.free_slots(), 0u);
    Equipment gear;
    gear.wear(EquipSlot::Weapon, "longsword");
    EXPECT_EQ(gear.equip_from(bag, 1, catalog), Equipment::Result::Done);
    EXPECT_EQ(bag.at(1)->item_id, "longsword");
}

TEST(Equipment, FailedEquipChangesNothing) {
    const auto catalog = make_catalog();
    Inventory bag;
    ASSERT_TRUE(bag.try_add(catalog, "potion", 5));
    Equipment gear;
    gear.wear(EquipSlot::Weapon, "longsword");
    EXPECT_EQ(gear.equip_from(bag, 0, catalog), Equipment::Result::NotEquipment);  // 물약은 못 껴요
    EXPECT_EQ(gear.equip_from(bag, 3, catalog), Equipment::Result::EmptySlot);     // 빈 칸
    EXPECT_EQ(gear.equip_from(bag, 99, catalog), Equipment::Result::EmptySlot);    // 밖
    EXPECT_EQ(gear.worn(EquipSlot::Weapon), "longsword");
    EXPECT_EQ(bag.at(0)->count, 5);
    EXPECT_EQ(bag.free_slots(), bag.slot_count() - 1);
}

TEST(Equipment, UnequipNeedsRoomInTheBag) {
    const auto catalog = make_catalog();
    Inventory full(1);
    ASSERT_TRUE(full.try_add(catalog, "potion", 1));
    Equipment gear;
    EXPECT_EQ(gear.unequip(EquipSlot::Weapon, full, catalog), Equipment::Result::NothingWorn);
    gear.wear(EquipSlot::Weapon, "dagger");
    EXPECT_EQ(gear.unequip(EquipSlot::Weapon, full, catalog), Equipment::Result::BagFull);
    EXPECT_EQ(gear.worn(EquipSlot::Weapon), "dagger");  // 못 벗었으면 그대로 끼고 있어요

    Inventory roomy;
    EXPECT_EQ(gear.unequip(EquipSlot::Weapon, roomy, catalog), Equipment::Result::Done);
    EXPECT_TRUE(gear.worn(EquipSlot::Weapon).empty());
    EXPECT_EQ(roomy.count_of("dagger"), 1);
}

}  // namespace
