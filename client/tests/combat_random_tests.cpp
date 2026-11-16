// 이 파일은 무엇을 하나요?
// 싸움 주사위(CombatRandom)를 숙제 검사(테스트)해요.
// - 같은 씨앗이면 같은 값, 다른 씨앗이면 다른 값
// - 값은 늘 0 이상 1 미만
// - 확률 0은 절대, 1은 언제나. 5%는 많이 굴리면 5% 근처
#include "game/combat/combat_random.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::CombatRandom;

TEST(CombatRandom, SameSeedSameSequence) {
    CombatRandom a(42);
    CombatRandom b(42);
    CombatRandom c(43);
    bool differs = false;
    for (int i = 0; i < 100; ++i) {
        const float va = a.next_unit();
        EXPECT_EQ(va, b.next_unit());
        differs = differs || va != c.next_unit();
    }
    EXPECT_TRUE(differs);
}

TEST(CombatRandom, StaysInUnitRange) {
    CombatRandom random(7);
    for (int i = 0; i < 10000; ++i) {
        const float v = random.next_unit();
        ASSERT_GE(v, 0.0f);
        ASSERT_LT(v, 1.0f);
    }
}

TEST(CombatRandom, RollEdgesAndRate) {
    CombatRandom random(2026);
    int crits = 0;
    for (int i = 0; i < 1000; ++i) {
        EXPECT_FALSE(random.roll(0.0f));
        EXPECT_TRUE(random.roll(1.0f));
        if (random.roll(0.05f)) ++crits;
    }
    EXPECT_GT(crits, 25);  // 1000번 중 50번 근처 (아주 넓게 잡았어요)
    EXPECT_LT(crits, 80);
}

// 확률과 상관없이 한 번씩 굴려서, 확률을 바꿔도 뒤 순서가 같아요.
TEST(CombatRandom, EveryRollAdvancesOnce) {
    CombatRandom a(9);
    CombatRandom b(9);
    (void)a.roll(0.0f);
    (void)b.roll(1.0f);
    EXPECT_EQ(a.next_unit(), b.next_unit());
}

}  // namespace
