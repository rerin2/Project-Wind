// 이 파일은 무엇을 하나요?
// 능력치 묶음 검사 (13a): 시작값, 레벨 = 합 − 49, 1~60 범위 경계.
#include "wind/progression/stats.hpp"

#include <gtest/gtest.h>

using wind::progression::StatId;
using wind::progression::Stats;

TEST(Stats, StartsAllTenAtLevelOne) {
    const Stats stats;
    for (const StatId id : wind::progression::kAllStats) EXPECT_EQ(stats[id], 10);
    EXPECT_EQ(wind::progression::level_of(stats), 1);
}

TEST(Stats, LevelIsSumMinus49) {
    Stats stats;
    stats[StatId::Strength] = 13;  // 힘 +3
    stats[StatId::Vigor] = 12;     // 생명력 +2
    EXPECT_EQ(wind::progression::level_of(stats), 6);
}

TEST(Stats, AllSixtyIsLevel251) {
    Stats stats;
    stats.values.fill(60);
    EXPECT_EQ(wind::progression::level_of(stats), 251);
    EXPECT_EQ(wind::progression::kMaxLevel, 251);
}

TEST(Stats, RangeIsOneToSixty) {
    Stats stats;
    stats.values.fill(1);
    EXPECT_TRUE(wind::progression::stats_in_range(stats));   // 1은 돼요
    stats[StatId::Mind] = 0;
    EXPECT_FALSE(wind::progression::stats_in_range(stats));  // 0은 안 돼요
    stats.values.fill(60);
    EXPECT_TRUE(wind::progression::stats_in_range(stats));   // 60은 돼요
    stats[StatId::Intelligence] = 61;
    EXPECT_FALSE(wind::progression::stats_in_range(stats));  // 61은 안 돼요
}

TEST(Stats, KeysMatchSpec) {
    EXPECT_EQ(wind::progression::stat_key(StatId::Vigor), "vigor");
    EXPECT_EQ(wind::progression::stat_key(StatId::Intelligence), "intelligence");
}
