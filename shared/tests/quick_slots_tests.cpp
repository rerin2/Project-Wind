// 이 파일은 무엇을 하나요?
// 단축 아이템 칸(15f)을 숙제 검사(테스트)해요: 쓸 수 있는 아이템만 등록, 다른 칸에서 옮기기,
// 같은 칸에 한 번 더면 풀기, 가방의 첫 묶음 찾기, 0개가 돼도 등록은 남음.
#include "wind/items/quick_slots.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::items::QuickSlots;

struct Fixture {
    wind::items::ItemCatalog catalog;
    wind::items::Inventory bag;
    QuickSlots quick;
    Fixture() {
        wind::items::ItemDef potion;
        potion.id = "potion";
        potion.max_stack = 2;  // 두 개씩 묶여서 칸이 여러 개가 돼요
        potion.kind = wind::items::ItemKind::Consumable;
        potion.use = wind::items::UseEffect{50, 0};
        catalog.add(potion);
        wind::items::ItemDef jelly;  // 재료: 쓸 수 없어요
        jelly.id = "jelly";
        jelly.max_stack = 99;
        catalog.add(jelly);
        bag.try_add(catalog, "jelly", 1);   // 0번 칸
        bag.try_add(catalog, "potion", 3);  // 1번 칸 (2개), 2번 칸 (1개)
    }
};

TEST(QuickSlots, OnlyUsableItemsCanBeAssigned) {
    Fixture f;
    EXPECT_EQ(f.quick.assign(0, "jelly", f.catalog), QuickSlots::Result::NotUsable);
    EXPECT_EQ(f.quick.assign(0, "nothing", f.catalog), QuickSlots::Result::NotUsable);
    EXPECT_EQ(f.quick.assign(4, "potion", f.catalog), QuickSlots::Result::BadSlot);
    EXPECT_TRUE(f.quick.at(0).empty());
    EXPECT_EQ(f.quick.assign(0, "potion", f.catalog), QuickSlots::Result::Assigned);
    EXPECT_EQ(f.quick.at(0), "potion");
}

// 한 아이템은 한 칸에만: 다른 칸에 등록하면 옮겨 가요. 같은 칸에 한 번 더면 풀려요
TEST(QuickSlots, MovesBetweenSlotsAndTogglesOff) {
    Fixture f;
    f.quick.assign(0, "potion", f.catalog);
    EXPECT_EQ(f.quick.assign(2, "potion", f.catalog), QuickSlots::Result::Assigned);
    EXPECT_TRUE(f.quick.at(0).empty());
    EXPECT_EQ(f.quick.slot_of("potion"), 2u);
    EXPECT_EQ(f.quick.assign(2, "potion", f.catalog), QuickSlots::Result::Cleared);
    EXPECT_FALSE(f.quick.slot_of("potion").has_value());
}

// 쓰기: 가방의 첫 묶음. 다 쓰면 찾을 수 없지만 등록은 남아요 (개발자와 정함)
TEST(QuickSlots, FindsTheFirstStackAndKeepsTheSlotWhenEmpty) {
    Fixture f;
    f.quick.assign(1, "potion", f.catalog);
    EXPECT_EQ(f.quick.stack_for(1, f.bag), 1u);
    EXPECT_FALSE(f.quick.stack_for(0, f.bag).has_value());  // 빈 칸
    ASSERT_TRUE(f.bag.try_remove("potion", 3));
    EXPECT_FALSE(f.quick.stack_for(1, f.bag).has_value());
    EXPECT_EQ(f.quick.at(1), "potion");
    f.bag.try_add(f.catalog, "potion", 1);  // 다시 주우면 바로
    EXPECT_TRUE(f.quick.stack_for(1, f.bag).has_value());
}

}  // namespace
