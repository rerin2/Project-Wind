// 이 파일은 무엇을 하나요?
// 피해 계산 순수 함수의 경계값을 숙제 검사(테스트)해요. (명세 9.3)
#include "wind/combat/damage.hpp"

#include <gtest/gtest.h>

#include <limits>

namespace {

using wind::combat::compute_damage;
using wind::combat::DamageInput;
using wind::combat::DamageRules;
using wind::combat::physical_attack_power;

// 기본: 공격력 14 × 위력 1.0 - 방어 4 = 10
TEST(Damage, AttackTimesPowerMinusDefense) {
    const auto r = compute_damage({.attack_power = 14, .power = 1.0f, .defense = 4});
    EXPECT_TRUE(r.hit);
    EXPECT_EQ(r.amount, 10);
}

// 소수점은 버려요: 14 × 1.1 = 15.4 -> 15
TEST(Damage, RoundsDown) {
    EXPECT_EQ(compute_damage({.attack_power = 14, .power = 1.1f}).amount, 15);
}

// 방어력이 더 커도 최소 피해(1)는 들어가요. 0이나 음수 피해는 없어요.
TEST(Damage, NeverBelowMinimum) {
    EXPECT_EQ(compute_damage({.attack_power = 5, .power = 1.0f, .defense = 50}).amount, 1);
    EXPECT_EQ(compute_damage({.attack_power = 0, .power = 1.0f}).amount, 1);
    EXPECT_EQ(compute_damage({.attack_power = 5, .defense = 50}, {.minimum = 3}).amount, 3);
}

// 무적이면 맞지 않아요. 최소 피해도 들어가지 않아요. (무적 검사가 가장 먼저)
TEST(Damage, InvulnerableTakesNothing) {
    const auto r = compute_damage({.attack_power = 100, .power = 2.0f, .invulnerable = true});
    EXPECT_FALSE(r.hit);
    EXPECT_EQ(r.amount, 0);
}

// 치명타 배수는 방어를 뺀 다음에 곱해요: (20 - 4) × 1.5 = 24
TEST(Damage, CriticalMultipliesAfterDefense) {
    EXPECT_EQ(compute_damage({.attack_power = 20, .defense = 4, .critical = true}).amount, 24);
    // 최소 피해에도 곱해져요: max(1, -) × 1.5 = 1.5 -> 1
    EXPECT_EQ(compute_damage({.attack_power = 1, .defense = 9, .critical = true}).amount, 1);
}

// 아주 큰 수도 정수 넘침 없이 최대값에서 멈춰요.
TEST(Damage, ClampsToMaximum) {
    const float huge = std::numeric_limits<float>::max();
    EXPECT_EQ(compute_damage({.attack_power = huge, .power = 10.0f, .critical = true}).amount,
              DamageRules{}.maximum);
    EXPECT_EQ(compute_damage({.attack_power = 500}, {.maximum = 100}).amount, 100);
}

// 이상한 숫자(음수, NaN, 무한대)는 0으로 봐요. 음수 방어가 피해를 늘리지 못해요.
TEST(Damage, BadNumbersCountAsZero) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    EXPECT_EQ(compute_damage({.attack_power = 10, .defense = -100}).amount, 10);
    EXPECT_EQ(compute_damage({.attack_power = nan}).amount, 1);
    EXPECT_EQ(compute_damage({.attack_power = inf}).amount, 1);
    EXPECT_EQ(compute_damage({.attack_power = -30}).amount, 1);
    EXPECT_EQ(compute_damage({.attack_power = 10, .power = -2.0f}).amount, 1);
}

// 규칙 숫자가 뒤집혀도(최소 > 최대) 안전하게 최소를 따라요.
TEST(Damage, BrokenRulesStaySafe) {
    EXPECT_EQ(compute_damage({.attack_power = 50}, {.minimum = 5, .maximum = 2}).amount, 5);
    EXPECT_EQ(compute_damage({.attack_power = 50}, {.minimum = -5, .maximum = 10}).amount, 10);
}

// 물리 공격력 = 기본 × (1 + 힘 보정 × 힘 보너스). 시작 힘 10은 보너스 0이라 롱소드 14 그대로.
TEST(Damage, PhysicalAttackPower) {
    EXPECT_FLOAT_EQ(physical_attack_power(14, 0.8f, 0.0f), 14.0f);
    EXPECT_FLOAT_EQ(physical_attack_power(14, 0.8f, 0.5f), 14.0f * 1.4f);
    EXPECT_FLOAT_EQ(physical_attack_power(14, 0.8f, -1.0f), 14.0f);
    EXPECT_FLOAT_EQ(physical_attack_power(-3, 0.8f, 1.0f), 0.0f);
}

}  // namespace
