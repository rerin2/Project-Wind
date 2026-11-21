// 이 파일은 무엇을 하나요?
// 주인공 HP 통(Health)을 숙제 검사(테스트)해요. (11c, 12b)
// - 가득 찬 채 시작하고, 맞으면 실제로 깎인 양을 돌려줘요
// - 12b 규칙: 기본은 0까지 내려가고, 0이면 empty예요
// - floor를 정하면 그 아래로는 안 내려가요 (연습용 칸)
// - 0·음수 피해는 무시하고, 가득 채우기가 돼요
#include "game/entities/health.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::Health;
using wind::game::HealthTuning;

TEST(Health, StartsFullAndTakesDamage) {
    Health health(HealthTuning{});
    EXPECT_EQ(health.current(), 100);
    EXPECT_FALSE(health.empty());
    EXPECT_EQ(health.take(4), 4);
    EXPECT_EQ(health.current(), 96);
    EXPECT_FLOAT_EQ(health.fraction(), 0.96f);
}

// 12b: 기본 규칙은 0까지. 넘치는 피해는 남은 HP만큼만 깎여요.
TEST(Health, DefaultReachesZeroAndIsEmpty) {
    Health health(HealthTuning{});
    EXPECT_EQ(health.take(98), 98);   // 100 -> 2
    EXPECT_EQ(health.take(10), 2);    // 2 -> 0: 실제로는 2만 깎여요
    EXPECT_EQ(health.current(), 0);
    EXPECT_TRUE(health.empty());
    EXPECT_FLOAT_EQ(health.fraction(), 0.0f);
    EXPECT_EQ(health.take(10), 0);    // 0에서는 더 안 깎여요 (음수가 되지 않아요)
    EXPECT_EQ(health.current(), 0);
}

TEST(Health, FloorStopsDamage) {
    Health health(HealthTuning{100, 1});  // 바닥 1 (11c 때의 임시 규칙)
    EXPECT_EQ(health.take(150), 99);
    EXPECT_EQ(health.current(), 1);
    EXPECT_FALSE(health.empty());
}

TEST(Health, IgnoresNonPositiveDamageAndRefills) {
    Health health(HealthTuning{});
    EXPECT_EQ(health.take(0), 0);
    EXPECT_EQ(health.take(-5), 0);    // 음수 피해로 HP가 늘지 않아요
    EXPECT_EQ(health.current(), 100);
    (void)health.take(100);
    EXPECT_TRUE(health.empty());
    health.refill();
    EXPECT_EQ(health.current(), 100);
    EXPECT_FALSE(health.empty());
}

}  // namespace
