// 이 파일은 무엇을 하나요?
// 스태미나 규칙(명세 7.2.1)을 숙제 검사(테스트)해요.
// - 0~최대 밖으로 나가지 않아요 (경계값)
// - 쓰고 나면 0.6초 기다린 뒤 1초에 30씩 차요
// - 0이 되면 지침: 25까지 차기 전에는 달리기·구르기 못 해요
#include "game/entities/stamina.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::Stamina;
using wind::game::StaminaTuning;

TEST(Stamina, StartsFullAndNeverGoesOutOfRange) {
    Stamina stamina(StaminaTuning{});
    EXPECT_FLOAT_EQ(stamina.current(), 100.0f);
    EXPECT_FLOAT_EQ(stamina.fraction(), 1.0f);
    stamina.spend(1000.0f);  // 너무 많이 써도 0에서 멈춰요
    EXPECT_FLOAT_EQ(stamina.current(), 0.0f);
    stamina.spend(-50.0f);   // 음수로 쓰는 것은 무시해요 (몰래 채우기 막기)
    EXPECT_FLOAT_EQ(stamina.current(), 0.0f);
    stamina.update(100.0f);  // 오래 쉬어도 최대에서 멈춰요
    EXPECT_FLOAT_EQ(stamina.current(), 100.0f);
}

// 쓰고 0.5초 동안은 안 차고, 0.6초가 지나면 차기 시작해요. 1초 더 쉬면 30 찹니다.
TEST(Stamina, RegeneratesAfterDelay) {
    Stamina stamina(StaminaTuning{});
    stamina.spend(60.0f);
    stamina.update(0.5f);
    EXPECT_FLOAT_EQ(stamina.current(), 40.0f);
    stamina.update(0.1f);
    EXPECT_NEAR(stamina.current(), 40.0f, 1e-3f);
    stamina.update(1.0f);
    EXPECT_NEAR(stamina.current(), 70.0f, 1e-3f);
}

// 한 박자가 기다리는 시간보다 길면, 남는 시간만큼은 채워요.
TEST(Stamina, LongStepSplitsWaitAndRegen) {
    Stamina stamina(StaminaTuning{});
    stamina.spend(50.0f);
    stamina.update(1.6f);  // 0.6초 기다림 + 1초 채움
    EXPECT_NEAR(stamina.current(), 80.0f, 1e-3f);
}

// 다 쓰면 지침: 24.9까지 차도 달리기·구르기 불가, 25가 되면 다시 돼요.
TEST(Stamina, ExhaustionLastsUntilThreshold) {
    Stamina stamina(StaminaTuning{});
    stamina.spend(100.0f);
    EXPECT_TRUE(stamina.exhausted());
    EXPECT_FALSE(stamina.can_run());
    EXPECT_FALSE(stamina.can_afford(1.0f));
    stamina.update(0.6f + 24.9f / 30.0f);
    EXPECT_TRUE(stamina.exhausted());
    EXPECT_FALSE(stamina.can_run());
    stamina.update(0.01f);
    EXPECT_FALSE(stamina.exhausted());
    EXPECT_TRUE(stamina.can_run());
    EXPECT_TRUE(stamina.can_afford(25.0f));
}

// 지침이 아니어도 스태미나가 모자라면 구르기(25)는 못 해요. 달리기는 조금만 있어도 돼요.
TEST(Stamina, CanAffordNeedsFullCost) {
    Stamina stamina(StaminaTuning{});
    stamina.spend(80.0f);  // 20 남음
    EXPECT_FALSE(stamina.exhausted());
    EXPECT_FALSE(stamina.can_afford(25.0f));
    EXPECT_TRUE(stamina.can_run());
}

TEST(Stamina, RefillClearsEverything) {
    Stamina stamina(StaminaTuning{});
    stamina.spend(100.0f);
    stamina.refill();
    EXPECT_FLOAT_EQ(stamina.current(), 100.0f);
    EXPECT_FALSE(stamina.exhausted());
}

}  // namespace
