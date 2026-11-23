// 이 파일은 무엇을 하나요?
// 맵마다 바닥 아이템을 따로 보관하는지 숙제 검사(테스트)해요. (14b: 맵을 떠나도 남아요)
#include "game/items/loot_store.hpp"

#include <gtest/gtest.h>

namespace {

TEST(LootStore, KeepsEachMapsItemsApart) {
    wind::game::LootStore store;
    auto& meadow = store.on("maps/meadow.json");
    meadow.drop({"jelly", 2}, {}, {});
    store.on("maps/forest.json").drop({"wing", 1}, {}, {});
    for (int i = 0; i < 20; ++i) (void)store.on("maps/other_" + std::to_string(i) + ".json");  // 다른 맵이 많아져도
    // 같은 맵을 다시 열면 같은 목록이고, 참조도 그대로예요
    EXPECT_EQ(&store.on("maps/meadow.json"), &meadow);
    ASSERT_EQ(meadow.items().size(), 1u);
    EXPECT_EQ(meadow.items()[0].stack.item_id, "jelly");
    ASSERT_EQ(store.on("maps/forest.json").items().size(), 1u);
    EXPECT_TRUE(store.on("maps/cave.json").items().empty());  // 처음 가는 맵은 비어 있어요
}

}  // namespace
