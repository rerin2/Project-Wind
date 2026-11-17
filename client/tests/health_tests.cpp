// 이 파일은 무엇을 하나요?
// 주인공 HP 통(Health)을 숙제 검사(테스트)해요. (11c)
// - 가득 찬 채 시작하고, 맞으면 실제로 깎인 양을 돌려줘요
// - 임시 규칙: 1 아래로는 안 내려가요 (사망은 12단계)
// - 0·음수 피해는 무시하고, 가득 채우기가 돼요
#include "game/entities/health.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::Health;
using wind::game::HealthTuning;

TEST(Health, StartsFullAndTakesDamage) {
    Health health(HealthTuning{});
    EXPECT_EQ(health.current(), 100);
    EXPECT_EQ(health.take(4), 4);
    EXPECT_EQ(health.current(), 96);
    EXPECT_FLOAT_EQ(health.fraction(), 0.96f);
}

TEST(Health, NeverDropsBelowFloor) {
    Health health(HealthTuning{});
    EXPECT_EQ(health.take(98), 98);   // 100 -> 2
    EXPECT_EQ(health.take(10), 1);    // 2 -> 1: 실제로는 1만 깎여요
    EXPECT_EQ(health.current(), 1);
    EXPECT_EQ(health.take(10), 0);    // 1에서는 더 안 깎여요 (11c 임시 규칙)
    EXPECT_EQ(health.current(), 1);
}

TEST(Health, IgnoresNonPositiveDamageAndRefills) {
    Health health(HealthTuning{});
    EXPECT_EQ(health.take(0), 0);
    EXPECT_EQ(health.take(-5), 0);    // 음수 피해로 HP가 늘지 않아요
    EXPECT_EQ(health.current(), 100);
    (void)health.take(50);
    health.refill();
    EXPECT_EQ(health.current(), 100);
}

TEST(Health, ZeroFloorReachesZero) {
    Health health(HealthTuning{100, 0});  // 12단계 규칙: 0까지 내려가요
    EXPECT_EQ(health.take(150), 100);
    EXPECT_EQ(health.current(), 0);
}

}  // namespace
