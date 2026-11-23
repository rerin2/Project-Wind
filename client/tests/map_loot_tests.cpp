// 이 파일은 무엇을 하나요?
// 맵 하나의 바닥 아이템 목록(번호, 지우기, 상한, 나이)을 숙제 검사(테스트)해요. (14b)
#include "game/items/map_loot.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::MapLoot;

TEST(MapLoot, DropGivesNewIdsAndRemoveWorksOnce) {
    MapLoot loot;
    const auto a = loot.drop({"jelly", 1}, {0.0f, 0.0f}, {10.0f, 0.0f});
    const auto b = loot.drop({"jelly", 1}, {0.0f, 0.0f}, {20.0f, 0.0f});
    EXPECT_NE(a, b);
    ASSERT_EQ(loot.items().size(), 2u);
    EXPECT_TRUE(loot.remove(a));
    EXPECT_FALSE(loot.remove(a));  // 같은 아이템은 두 번 못 지워요 (두 번 못 주워요)
    ASSERT_EQ(loot.items().size(), 1u);
    EXPECT_EQ(loot.items()[0].id, b);
    const auto c = loot.drop({"jelly", 1}, {}, {});
    EXPECT_NE(c, a);  // 지운 번호도 다시 쓰지 않아요
}

TEST(MapLoot, OldestGoesWhenOverTheLimit) {
    MapLoot loot;
    const auto first = loot.drop({"first", 1}, {}, {});
    for (std::size_t i = 1; i < MapLoot::kMaxItems; ++i) loot.drop({"x", 1}, {}, {});
    EXPECT_EQ(loot.items().size(), MapLoot::kMaxItems);
    loot.drop({"newest", 1}, {}, {});
    EXPECT_EQ(loot.items().size(), MapLoot::kMaxItems);  // 상한 그대로
    EXPECT_NE(loot.items().front().id, first);           // 가장 오래된 것이 사라졌어요
    EXPECT_EQ(loot.items().back().stack.item_id, "newest");
    EXPECT_EQ(loot.discarded(), 1u);
}

TEST(MapLoot, AgeAddsToEveryItem) {
    MapLoot loot;
    loot.drop({"a", 1}, {}, {});
    loot.age(0.25f);
    loot.drop({"b", 1}, {}, {});
    loot.age(0.25f);
    EXPECT_FLOAT_EQ(loot.items()[0].age, 0.5f);
    EXPECT_FLOAT_EQ(loot.items()[1].age, 0.25f);
}

}  // namespace
