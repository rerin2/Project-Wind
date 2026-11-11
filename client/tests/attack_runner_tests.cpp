// 이 파일은 무엇을 하나요?
// 칼 휘두르기 진행(AttackRunner)을 숙제 검사해요. (명세 9.2, 9단계 완료 조건)
// - 준비 -> 판정 -> 후딜 순서
// - 취소 구간에서만 다음 콤보, 3타 다음에는 1타부터
// - 한 번 휘두를 때 같은 대상은 한 번만 맞아요
#include "game/combat/attack_runner.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using wind::combat::AttackData;
using wind::combat::Weapon;
using wind::game::AttackRunner;
using Phase = AttackRunner::Phase;

constexpr float kDt = 1.0f / 60.0f;

// 롱소드 1타와 같은 박자: 0.08 / 0.10 / 0.18, 취소 구간 0.18~0.36
AttackData make_attack(std::string id) {
    AttackData a;
    a.id = std::move(id);
    a.windup = 0.08f;
    a.active = 0.10f;
    a.recovery = 0.18f;
    a.cancel_start = 0.18f;
    a.cancel_end = 0.36f;
    return a;
}

Weapon make_weapon(int hits = 3) {
    Weapon w;
    w.id = "sword";
    for (int i = 0; i < hits; ++i) {
        w.combo.push_back(make_attack("hit" + std::to_string(i + 1)));
    }
    return w;
}

void advance(AttackRunner& runner, float seconds) {
    // 박자로 나눠서 흘려보내요 (진짜 게임처럼)
    for (float t = 0.0f; t + 1e-6f < seconds; t += kDt) {
        runner.step(kDt);
    }
}

TEST(AttackRunner, PhasesInOrderThenEnds) {
    AttackRunner runner(make_weapon());
    EXPECT_EQ(runner.phase(), Phase::None);
    ASSERT_TRUE(runner.start());
    EXPECT_EQ(runner.combo_index(), 0);
    EXPECT_EQ(runner.phase(), Phase::Windup);
    runner.step(0.09f);
    EXPECT_EQ(runner.phase(), Phase::Active);
    runner.step(0.10f);
    EXPECT_EQ(runner.phase(), Phase::Recovery);
    runner.step(0.20f);  // 0.39초: 끝
    EXPECT_EQ(runner.phase(), Phase::None);
    EXPECT_EQ(runner.combo_index(), -1);
}

// 취소 구간 전에는 다음 공격을 시작할 수 없어요.
TEST(AttackRunner, NextComboOnlyInCancelWindow) {
    AttackRunner runner(make_weapon());
    runner.start();
    runner.step(0.10f);  // 판정 중
    EXPECT_FALSE(runner.in_cancel_window());
    EXPECT_FALSE(runner.can_start());
    EXPECT_FALSE(runner.start());
    EXPECT_EQ(runner.combo_index(), 0);

    runner.step(0.10f);  // 0.20초: 취소 구간
    EXPECT_TRUE(runner.in_cancel_window());
    ASSERT_TRUE(runner.start());
    EXPECT_EQ(runner.combo_index(), 1);
    EXPECT_FLOAT_EQ(runner.elapsed(), 0.0f);
}

// 1 -> 2 -> 3타로 이어지고, 3타의 취소 구간에서는 이어갈 공격이 없어요.
// 3타가 끝나면 다음 공격은 1타부터예요.
TEST(AttackRunner, ThreeHitComboThenRestarts) {
    AttackRunner runner(make_weapon(3));
    runner.start();
    for (int hit = 1; hit < 3; ++hit) {
        advance(runner, 0.20f);
        ASSERT_TRUE(runner.start()) << "hit " << hit + 1;
        EXPECT_EQ(runner.combo_index(), hit);
    }
    advance(runner, 0.20f);
    EXPECT_TRUE(runner.in_cancel_window());
    EXPECT_FALSE(runner.can_start());  // 4타는 없어요
    advance(runner, 0.20f);
    EXPECT_FALSE(runner.busy());
    ASSERT_TRUE(runner.start());
    EXPECT_EQ(runner.combo_index(), 0);
}

// 한 번 휘두를 때 같은 대상은 한 번만 맞아요. 다른 대상은 따로 맞아요.
TEST(AttackRunner, SameTargetHitOncePerSwing) {
    AttackRunner runner(make_weapon());
    runner.start();
    EXPECT_FALSE(runner.register_hit(7));  // 준비 중에는 안 맞아요
    runner.step(0.09f);
    ASSERT_EQ(runner.phase(), Phase::Active);
    EXPECT_TRUE(runner.register_hit(7));
    for (int i = 0; i < 5; ++i) {  // 판정 구간 동안 계속 겹쳐 있어도
        runner.step(kDt * 0.5f);
        EXPECT_FALSE(runner.register_hit(7));
    }
    EXPECT_TRUE(runner.register_hit(8));
    runner.step(0.10f);
    EXPECT_FALSE(runner.register_hit(9));  // 후딜에는 안 맞아요
}

// 다음 콤보(새 휘두르기)에서는 같은 대상을 다시 맞힐 수 있어요.
TEST(AttackRunner, NextSwingCanHitAgain) {
    AttackRunner runner(make_weapon());
    runner.start();
    runner.step(0.09f);
    EXPECT_TRUE(runner.register_hit(7));
    runner.step(0.11f);  // 0.20초: 취소 구간
    ASSERT_TRUE(runner.start());
    runner.step(0.09f);
    EXPECT_TRUE(runner.register_hit(7));
}

// 취소하면 바로 쉬는 상태, 콤보도 처음으로
TEST(AttackRunner, CancelResets) {
    AttackRunner runner(make_weapon());
    runner.start();
    runner.step(0.2f);
    runner.start();
    runner.cancel();
    EXPECT_FALSE(runner.busy());
    ASSERT_TRUE(runner.start());
    EXPECT_EQ(runner.combo_index(), 0);
}

// 무기가 없으면(맨손) 공격이 나가지 않아요.
TEST(AttackRunner, NoWeaponNoAttack) {
    AttackRunner runner;
    EXPECT_FALSE(runner.has_weapon());
    EXPECT_FALSE(runner.start());
    runner.step(1.0f);  // 아무 일도 없어요
    EXPECT_EQ(runner.phase(), Phase::None);
}

}  // namespace
