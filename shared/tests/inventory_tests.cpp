// 이 파일은 무엇을 하나요?
// 가방 규칙(중첩, 가득 참, 전부 성공 또는 전부 실패, 정렬)을 숙제 검사(테스트)해요. (명세 10.2, 14a)
#include "wind/items/inventory.hpp"

#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::items::Inventory;
using wind::items::ItemCatalog;
using wind::items::ItemDef;
using wind::items::ItemStack;

// 물약(20개씩), 젤리(99개씩), 검(1개씩) 세 가지가 든 도감
ItemCatalog make_catalog() {
    ItemCatalog catalog;
    auto add = [&catalog](const char* id, std::int32_t max_stack) {
        ItemDef def;
        def.id = id;
        def.name = id;
        def.max_stack = max_stack;
        catalog.add(def);
    };
    add("potion", 20);
    add("jelly", 99);
    add("sword", 1);
    return catalog;
}

// 가방 칸들을 그대로 복사해요 ("아무것도 안 바뀜" 비교용)
std::vector<std::optional<ItemStack>> snapshot(const Inventory& bag) {
    std::vector<std::optional<ItemStack>> out;
    for (std::size_t i = 0; i < bag.slot_count(); ++i) out.push_back(bag.at(i));
    return out;
}

TEST(Inventory, StartsWith24EmptySlots) {
    const Inventory bag;
    EXPECT_EQ(bag.slot_count(), 24u);
    EXPECT_EQ(bag.free_slots(), 24u);
    EXPECT_FALSE(bag.at(0).has_value());
    EXPECT_FALSE(bag.at(999).has_value());  // 밖의 칸 번호도 멈추지 않고 빈 칸
}

TEST(Inventory, StacksSameItemBeforeUsingNewSlots) {
    const auto catalog = make_catalog();
    Inventory bag(4);
    ASSERT_TRUE(bag.try_add(catalog, "potion", 15));
    ASSERT_TRUE(bag.try_add(catalog, "potion", 10));  // 5개는 첫 칸, 5개는 새 칸
    EXPECT_EQ(bag.at(0), (ItemStack{"potion", 20}));
    EXPECT_EQ(bag.at(1), (ItemStack{"potion", 5}));
    EXPECT_EQ(bag.count_of("potion"), 25);
    EXPECT_EQ(bag.free_slots(), 2u);
}

TEST(Inventory, NonStackingItemsTakeOneSlotEach) {
    const auto catalog = make_catalog();
    Inventory bag(3);
    ASSERT_TRUE(bag.try_add(catalog, "sword", 2));
    EXPECT_EQ(bag.at(0), (ItemStack{"sword", 1}));
    EXPECT_EQ(bag.at(1), (ItemStack{"sword", 1}));
    EXPECT_EQ(bag.free_slots(), 1u);
}

TEST(Inventory, FullBagRejectsWholeAddAndChangesNothing) {
    const auto catalog = make_catalog();
    Inventory bag(2);
    ASSERT_TRUE(bag.try_add(catalog, "potion", 18));
    ASSERT_TRUE(bag.try_add(catalog, "sword", 1));
    const auto before = snapshot(bag);
    // 물약 3개: 첫 칸에 2개 자리뿐이라 다 못 넣어요 → 하나도 안 넣어요
    EXPECT_FALSE(bag.can_add(catalog, "potion", 3));
    EXPECT_FALSE(bag.try_add(catalog, "potion", 3));
    EXPECT_EQ(snapshot(bag), before);
    EXPECT_FALSE(bag.try_add(catalog, "jelly", 1));  // 빈 칸이 없어요
    EXPECT_EQ(snapshot(bag), before);
    EXPECT_TRUE(bag.try_add(catalog, "potion", 2));  // 딱 맞으면 들어가요
    EXPECT_EQ(bag.count_of("potion"), 20);
}

TEST(Inventory, RejectsUnknownItemsAndBadCounts) {
    const auto catalog = make_catalog();
    Inventory bag;
    EXPECT_FALSE(bag.try_add(catalog, "dragon_egg", 1));  // 도감에 없음
    EXPECT_FALSE(bag.try_add(catalog, "potion", 0));
    EXPECT_FALSE(bag.try_add(catalog, "potion", -3));
    EXPECT_EQ(bag.free_slots(), 24u);
}

TEST(Inventory, RemoveIsAllOrNothingAndTakesFromBackSlots) {
    const auto catalog = make_catalog();
    Inventory bag(4);
    ASSERT_TRUE(bag.try_add(catalog, "potion", 25));  // [20][5]
    const auto before = snapshot(bag);
    EXPECT_FALSE(bag.try_remove("potion", 26));  // 모자라요 → 그대로
    EXPECT_EQ(snapshot(bag), before);
    EXPECT_FALSE(bag.try_remove("potion", 0));
    EXPECT_FALSE(bag.try_remove("jelly", 1));
    ASSERT_TRUE(bag.try_remove("potion", 7));  // 뒤 칸 5개를 다 쓰고 앞 칸에서 2개
    EXPECT_EQ(bag.at(0), (ItemStack{"potion", 18}));
    EXPECT_FALSE(bag.at(1).has_value());  // 다 쓴 칸은 비어요
}

TEST(Inventory, TakeAtTakesFromOneSlotOrNothing) {
    const auto catalog = make_catalog();
    Inventory bag(3);
    ASSERT_TRUE(bag.try_add(catalog, "jelly", 5));
    EXPECT_FALSE(bag.take_at(0, 6).has_value());  // 가진 것보다 많이
    EXPECT_FALSE(bag.take_at(0, 0).has_value());
    EXPECT_FALSE(bag.take_at(1, 1).has_value());  // 빈 칸
    EXPECT_FALSE(bag.take_at(50, 1).has_value());  // 밖의 칸
    EXPECT_EQ(bag.count_of("jelly"), 5);
    const auto taken = bag.take_at(0, 2);
    ASSERT_TRUE(taken.has_value());
    EXPECT_EQ(*taken, (ItemStack{"jelly", 2}));
    EXPECT_EQ(bag.count_of("jelly"), 3);
    ASSERT_TRUE(bag.take_at(0, 3).has_value());
    EXPECT_FALSE(bag.at(0).has_value());
}

TEST(Inventory, SortMergesStacksInCatalogOrder) {
    const auto catalog = make_catalog();
    Inventory bag(5);
    ASSERT_TRUE(bag.try_add(catalog, "sword", 1));
    ASSERT_TRUE(bag.try_add(catalog, "jelly", 3));
    ASSERT_TRUE(bag.try_add(catalog, "potion", 20));
    ASSERT_TRUE(bag.try_add(catalog, "potion", 15));  // [sword][jelly3][potion20][potion15]
    ASSERT_TRUE(bag.take_at(2, 10).has_value());     // [sword][jelly3][potion10][potion15]
    ASSERT_TRUE(bag.sort(catalog));
    // 도감 순서(potion, jelly, sword)이고 물약은 20+5로 다시 묶여요
    EXPECT_EQ(bag.at(0), (ItemStack{"potion", 20}));
    EXPECT_EQ(bag.at(1), (ItemStack{"potion", 5}));
    EXPECT_EQ(bag.at(2), (ItemStack{"jelly", 3}));
    EXPECT_EQ(bag.at(3), (ItemStack{"sword", 1}));
    EXPECT_FALSE(bag.at(4).has_value());
}

TEST(Inventory, SortKeepsItemsMissingFromCatalogAtTheBack) {
    auto catalog = make_catalog();
    Inventory bag(3);
    ItemCatalog old_catalog;  // 예전 도감에만 있던 아이템 (예: 옛 저장 파일)
    ItemDef relic;
    relic.id = "relic";
    relic.max_stack = 5;
    old_catalog.add(relic);
    ASSERT_TRUE(bag.try_add(old_catalog, "relic", 2));
    ASSERT_TRUE(bag.try_add(catalog, "jelly", 1));
    ASSERT_TRUE(bag.sort(catalog));
    EXPECT_EQ(bag.at(0), (ItemStack{"jelly", 1}));
    EXPECT_EQ(bag.at(1), (ItemStack{"relic", 2}));  // 버리지 않고 뒤에 둬요
}

TEST(Inventory, ZeroSlotBagAcceptsNothing) {
    const auto catalog = make_catalog();
    Inventory bag(0);
    EXPECT_FALSE(bag.try_add(catalog, "potion", 1));
    EXPECT_TRUE(bag.sort(catalog));
}

}  // namespace

// 빈 칸에 그대로 놓기 (15a 장비 바꿔 끼우기): 빈 칸에만, 개수 1 이상만
TEST(Inventory, PlaceAtOnlyFillsAnEmptySlot) {
    const auto catalog = make_catalog();
    Inventory bag(2);
    ASSERT_TRUE(bag.try_add(catalog, "potion", 2));
    const auto before = snapshot(bag);
    EXPECT_FALSE(bag.place_at(0, ItemStack{"sword", 1}));  // 차 있어요
    EXPECT_FALSE(bag.place_at(5, ItemStack{"sword", 1}));  // 밖
    EXPECT_FALSE(bag.place_at(1, ItemStack{"sword", 0}));  // 0개
    EXPECT_EQ(snapshot(bag), before);
    EXPECT_TRUE(bag.place_at(1, ItemStack{"sword", 1}));
    EXPECT_EQ(bag.at(1)->item_id, "sword");
}
