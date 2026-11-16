// 이 파일은 무엇을 하나요?
// MP 규칙(명세 8.2)을 숙제 검사(테스트)해요.
// - 최대 50으로 시작하고, 0~최대 밖으로 나가지 않아요 (경계값)
// - 모자라면 하나도 안 줄고 실패해요 (전부 아니면 전부 안 함)
// - 1초에 2씩, 기다림 없이 바로 차요
#include "game/entities/mana.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::Mana;
using wind::game::ManaTuning;

TEST(Mana, StartsFullAtFifty) {
    const Mana mana(ManaTuning{});
    EXPECT_FLOAT_EQ(mana.current(), 50.0f);
    EXPECT_FLOAT_EQ(mana.max(), 50.0f);
    EXPECT_FLOAT_EQ(mana.fraction(), 1.0f);
}

// 딱 맞게 있으면 쓸 수 있고, 0.01이라도 모자라면 아무것도 안 바뀌어요.
TEST(Mana, SpendIsAllOrNothing) {
    Mana mana(ManaTuning{});
    EXPECT_TRUE(mana.try_spend(44.0f));  // 6 남음
    EXPECT_FLOAT_EQ(mana.current(), 6.0f);
    EXPECT_TRUE(mana.can_afford(6.0f));
    EXPECT_FALSE(mana.can_afford(6.01f));
    EXPECT_FALSE(mana.try_spend(7.0f));  // 모자람: 그대로 6
    EXPECT_FLOAT_EQ(mana.current(), 6.0f);
    EXPECT_TRUE(mana.try_spend(6.0f));   // 딱 맞음: 0
    EXPECT_FLOAT_EQ(mana.current(), 0.0f);
    EXPECT_FALSE(mana.try_spend(1.0f));  // 0에서는 못 써요
    EXPECT_FLOAT_EQ(mana.current(), 0.0f);
}

// 음수로 "쓰기"는 몰래 채우는 길이라 무시해요.
TEST(Mana, NegativeCostDoesNotRefill) {
    Mana mana(ManaTuning{});
    ASSERT_TRUE(mana.try_spend(20.0f));
    EXPECT_TRUE(mana.try_spend(-30.0f));
    EXPECT_FLOAT_EQ(mana.current(), 30.0f);
}

// 쓰자마자 차기 시작해요: 1초에 2, 최대 50에서 멈춤.
TEST(Mana, RegeneratesTwoPerSecondUpToMax) {
    Mana mana(ManaTuning{});
    ASSERT_TRUE(mana.try_spend(10.0f));
    mana.update(1.0f);
    EXPECT_NEAR(mana.current(), 42.0f, 1e-4f);
    mana.update(0.0f);   // 0초는 아무 일 없음
    mana.update(-5.0f);  // 거꾸로 가는 시간은 무시
    EXPECT_NEAR(mana.current(), 42.0f, 1e-4f);
    mana.update(100.0f);  // 오래 쉬어도 최대에서 멈춰요
    EXPECT_FLOAT_EQ(mana.current(), 50.0f);
}

// 정해진 박자(1/60초)로 3초 = 6 찹니다.
TEST(Mana, FixedStepsAddUp) {
    Mana mana(ManaTuning{});
    ASSERT_TRUE(mana.try_spend(50.0f));
    for (int i = 0; i < 180; ++i) {
        mana.update(1.0f / 60.0f);
    }
    EXPECT_NEAR(mana.current(), 6.0f, 1e-3f);
}

TEST(Mana, RefillFillsToMax) {
    Mana mana(ManaTuning{});
    ASSERT_TRUE(mana.try_spend(50.0f));
    mana.refill();
    EXPECT_FLOAT_EQ(mana.current(), 50.0f);
}

// 최대가 0이나 음수로 잘못 와도 나누기 오류 없이 0이에요.
TEST(Mana, ZeroOrNegativeMaxIsSafe) {
    ManaTuning tuning;
    tuning.max = -5.0f;
    Mana mana(tuning);
    EXPECT_FLOAT_EQ(mana.current(), 0.0f);
    EXPECT_FLOAT_EQ(mana.max(), 0.0f);
    EXPECT_FLOAT_EQ(mana.fraction(), 0.0f);
    mana.update(10.0f);
    EXPECT_FLOAT_EQ(mana.current(), 0.0f);
}

}  // namespace
