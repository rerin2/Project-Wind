// 이 파일은 무엇을 하나요?
// 드롭 표 굴리기(확률·개수·재현)와 도감 참조 검사를 숙제 검사(테스트)해요. (14b)
#include "game/items/drop_roll.hpp"

#include "game/combat/combat_random.hpp"

#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

#include <set>

namespace {

using wind::game::CombatRandom;
using wind::game::roll_drops;
using wind::items::DropEntry;

TEST(DropRoll, SureDropAlwaysComesWithCountInRange) {
    CombatRandom random(7);
    const std::vector<DropEntry> table{{"jelly", 1.0f, 1, 3}};
    std::set<int> seen;  // 나온 개수들
    for (int i = 0; i < 200; ++i) {
        const auto out = roll_drops(table, random);
        ASSERT_EQ(out.size(), 1u);
        EXPECT_EQ(out[0].item_id, "jelly");
        EXPECT_GE(out[0].count, 1);
        EXPECT_LE(out[0].count, 3);
        seen.insert(out[0].count);
    }
    EXPECT_EQ(seen.size(), 3u);  // 1, 2, 3이 모두 나와요 (끝값도 빠지지 않아요)
}

TEST(DropRoll, ChanceIsRoughlyRespected) {
    CombatRandom random(11);
    const std::vector<DropEntry> table{{"potion", 0.15f, 1, 1}};
    int drops = 0;
    for (int i = 0; i < 2000; ++i) drops += static_cast<int>(roll_drops(table, random).size());
    EXPECT_GT(drops, 220);  // 기대값 300 (15%)
    EXPECT_LT(drops, 380);
}

TEST(DropRoll, EachLineRollsOnItsOwnAndSameSeedSameResult) {
    const std::vector<DropEntry> table{{"a", 0.5f, 1, 2}, {"b", 0.5f, 1, 1}};
    CombatRandom first(42);
    CombatRandom second(42);
    for (int i = 0; i < 50; ++i) {
        const auto x = roll_drops(table, first);
        const auto y = roll_drops(table, second);
        EXPECT_EQ(x, y);  // 같은 씨앗이면 같은 드롭 (재현)
        EXPECT_LE(x.size(), 2u);
    }
}

TEST(DropRoll, EmptyTableDropsNothing) {
    CombatRandom random(1);
    EXPECT_TRUE(roll_drops({}, random).empty());
}

TEST(DropRoll, FindsDropsMissingFromCatalog) {
    wind::items::ItemCatalog catalog;
    wind::items::ItemDef jelly;
    jelly.id = "jelly";
    catalog.add(jelly);
    EXPECT_FALSE(wind::game::unknown_drop_item({{"jelly", 0.5f, 1, 1}}, catalog).has_value());
    const auto bad = wind::game::unknown_drop_item({{"jelly", 0.5f, 1, 1}, {"dragon_egg", 0.1f, 1, 1}}, catalog);
    ASSERT_TRUE(bad.has_value());
    EXPECT_EQ(*bad, "dragon_egg");
}

}  // namespace
