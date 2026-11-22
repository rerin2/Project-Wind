// 이 파일은 무엇을 하나요?
// 레벨업 비용 순수 함수 검사 (13a): 명세 8.3 예시 값, 레벨 1·250·251 경계, 여러 레벨 합.
#include "wind/progression/level_cost.hpp"

#include <gtest/gtest.h>

using wind::progression::LevelCostRules;

TEST(LevelCost, MatchesSpecExamples) {
    const LevelCostRules rules;  // 기본 100, 40, 6
    EXPECT_EQ(wind::progression::level_up_cost(1, rules), 100);
    EXPECT_EQ(wind::progression::level_up_cost(10, rules), 946);
    EXPECT_EQ(wind::progression::level_up_cost(30, rules), 6306);
}

TEST(LevelCost, LastLevelUpIs250To251) {
    const LevelCostRules rules;
    // 100 + 40·249 + 6·249² = 382,066
    EXPECT_EQ(wind::progression::level_up_cost(250, rules), 382066);
    EXPECT_FALSE(wind::progression::level_up_cost(251, rules).has_value());  // 251은 끝
    EXPECT_FALSE(wind::progression::level_up_cost(0, rules).has_value());    // 0은 없는 레벨
    EXPECT_FALSE(wind::progression::level_up_cost(-5, rules).has_value());
}

TEST(LevelCost, FloorsFractions) {
    const LevelCostRules rules{10.5, 0.25, 0.0};
    EXPECT_EQ(wind::progression::level_up_cost(1, rules), 10);  // 10.5 → 10
    EXPECT_EQ(wind::progression::level_up_cost(3, rules), 11);  // 11.0
}

TEST(LevelCost, CostBetweenAddsEachStep) {
    const LevelCostRules rules;
    EXPECT_EQ(wind::progression::cost_between(1, 1, rules), 0);
    EXPECT_EQ(wind::progression::cost_between(1, 2, rules), 100);
    EXPECT_EQ(wind::progression::cost_between(1, 3, rules), 100 + 146);  // 1→2, 2→3
    EXPECT_FALSE(wind::progression::cost_between(3, 2, rules).has_value());    // 거꾸로는 안 돼요
    EXPECT_FALSE(wind::progression::cost_between(1, 252, rules).has_value());  // 251 넘으면 안 돼요
    EXPECT_TRUE(wind::progression::cost_between(1, 251, rules).has_value());   // 끝까지는 돼요
}
