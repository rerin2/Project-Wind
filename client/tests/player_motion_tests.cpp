// 이 파일은 무엇을 하나요?
// 주인공 몸 움직임(PlayerMotion)이 스태미나 규칙대로 달리고 구르는지 숙제 검사해요. (8단계 완료 조건)
// - Shift를 누르고 움직이면 1.5배 빠르게 달리고, 1초에 25씩 스태미나를 써요
// - 제자리에서 Shift만 누르면 스태미나를 안 써요
// - 구르기는 25를 쓰고, 25가 안 되면 구르지 않아요
// - 다 쓰면 지침: 25까지 차야 다시 달리고 구를 수 있어요
#include "game/entities/player_motion.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using wind::engine::Vec2;
using wind::game::CharacterState;
using wind::game::Mover;
using wind::game::MovementTuning;
using wind::game::PlayerMotion;
using wind::game::StaminaTuning;

constexpr double kDt = 1.0 / 60.0;
constexpr Vec2 kRight{1.0f, 0.0f};

// 벽이 없는 아주 넓은 맵 (오래 달려도 벽에 안 닿게)
wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 400;
    map.height = 20;
    map.walkable.assign(static_cast<std::size_t>(400 * 20), 1);
    return map;
}

PlayerMotion make_player() {
    return PlayerMotion({100.0f, 300.0f}, wind::world::Direction::Right, MovementTuning{},
                        StaminaTuning{}, 0.15);
}

void run_for(PlayerMotion& player, int steps, Vec2 move, bool run, const wind::world::MapData& map) {
    for (int i = 0; i < steps; ++i) {
        player.step(kDt, move, run, map);
    }
}

TEST(PlayerMotion, RunningIsFasterAndCostsStamina) {
    const auto map = open_map();
    auto player = make_player();
    run_for(player, 60, kRight, true, map);  // 1초 달리기
    EXPECT_EQ(player.mover().state(), Mover::State::Running);
    EXPECT_NEAR(player.mover().speed(), 165.0f, 0.5f);
    EXPECT_NEAR(player.stamina().current(), 75.0f, 0.5f);  // 1초에 25
}

TEST(PlayerMotion, StandingWithShiftCostsNothing) {
    const auto map = open_map();
    auto player = make_player();
    run_for(player, 60, {0.0f, 0.0f}, true, map);
    EXPECT_EQ(player.mover().state(), Mover::State::Idle);
    EXPECT_FLOAT_EQ(player.stamina().current(), 100.0f);
}

TEST(PlayerMotion, DodgeCostsStaminaAndNeedsEnough) {
    const auto map = open_map();
    auto player = make_player();
    player.press_dodge();
    EXPECT_TRUE(player.step(kDt, kRight, false, map));
    EXPECT_NEAR(player.stamina().current(), 75.0f, 1e-3f);

    // 3초 달려서 거의 다 쓰면(지침) 버튼을 눌러도 구르지 않아요.
    run_for(player, 30, kRight, false, map);  // 구르기 끝나고 쉬는 시간도 지나게
    run_for(player, 200, kRight, true, map);
    EXPECT_TRUE(player.stamina().exhausted());
    player.press_dodge();
    EXPECT_FALSE(player.step(kDt, kRight, false, map));
}

// 지침이면 Shift를 눌러도 걷는 속도예요. 쉬어서 25가 되면 다시 달려요.
TEST(PlayerMotion, ExhaustedWalksUntilRecovered) {
    const auto map = open_map();
    auto player = make_player();
    // 1초에 25씩이라 4초(240박자) 만에 0이 돼요. 그 순간부터 시간을 세요.
    run_for(player, 240, kRight, true, map);
    // Shift를 계속 눌러도 지침이라 걷는 속도로 줄어요. (0이 된 뒤 30박자 = 0.5초, 아직 안 참)
    run_for(player, 30, kRight, true, map);
    EXPECT_TRUE(player.stamina().exhausted());
    EXPECT_EQ(player.mover().state(), Mover::State::Walking);
    EXPECT_NEAR(player.mover().speed(), 110.0f, 0.5f);

    // 걷는 동안에도 차요. 0.6초(36박자) 기다린 뒤 1초에 30씩 -> 25까지 약 50박자 더.
    run_for(player, 50, kRight, true, map);  // 0이 된 뒤 80박자: 약 22 -> 아직 지침
    EXPECT_TRUE(player.stamina().exhausted());
    run_for(player, 10, kRight, true, map);  // 90박자: 약 27 -> 다시 달릴 수 있어요
    EXPECT_FALSE(player.stamina().exhausted());
    run_for(player, 30, kRight, true, map);
    EXPECT_EQ(player.mover().state(), Mover::State::Running);
}

// ---- 9단계: 공격 (명세 7.2.1, 9.2) ----

wind::combat::Weapon make_sword() {
    wind::combat::Weapon w;
    w.id = "sword";
    w.stamina_per_attack = 14.0f;
    for (int i = 1; i <= 3; ++i) {
        wind::combat::AttackData a;
        a.id = "hit" + std::to_string(i);
        a.windup = 0.08f;
        a.active = 0.10f;
        a.recovery = 0.18f;
        a.cancel_start = 0.18f;
        a.cancel_end = 0.36f;
        a.hitbox = {18.0f, -8.0f, 28.0f, 24.0f};
        w.combo.push_back(a);
    }
    return w;
}

PlayerMotion make_fighter() {
    return PlayerMotion({100.0f, 300.0f}, wind::world::Direction::Right, MovementTuning{},
                        StaminaTuning{}, 0.15, make_sword());
}

// 휘두를 때마다 스태미나가 무기 값(14)만큼 줄어요.
TEST(PlayerMotion, AttackCostsStaminaPerSwing) {
    const auto map = open_map();
    auto player = make_fighter();
    player.press_attack();
    player.step(kDt, {}, false, map);
    EXPECT_TRUE(player.attack_started());
    EXPECT_EQ(player.state(), CharacterState::Attacking);
    EXPECT_NEAR(player.stamina().current(), 86.0f, 1e-3f);
    player.step(kDt, {}, false, map);
    EXPECT_FALSE(player.attack_started());  // 한 박자만 알려 줘요
}

// 스태미나가 0보다 많으면 모자라도 공격이 나가고 0이 돼요. 0(지침)이면 나가지 않아요.
TEST(PlayerMotion, AttackNeedsSomeStaminaAndCanEmptyIt) {
    const auto map = open_map();
    auto player = make_fighter();
    run_for(player, 228, kRight, true, map);  // 3.8초 달리기: 100 - 95 = 5 남음
    ASSERT_NEAR(player.stamina().current(), 5.0f, 0.5f);
    player.press_attack();
    player.step(kDt, {}, false, map);
    EXPECT_TRUE(player.attack_started());
    EXPECT_FLOAT_EQ(player.stamina().current(), 0.0f);
    EXPECT_TRUE(player.stamina().exhausted());

    run_for(player, 30, {}, false, map);  // 공격이 끝나도 아직 지침 (0.6초 기다리는 중)
    ASSERT_FALSE(player.attack().busy());
    player.press_attack();
    player.step(kDt, {}, false, map);
    EXPECT_FALSE(player.attack_started());
    EXPECT_NE(player.state(), CharacterState::Attacking);
}

// 1타 도중 일찍 누른 공격은 메모장이 기억했다가 취소 구간이 열리면 2타로 이어져요.
TEST(PlayerMotion, ComboFollowsBufferedPress) {
    const auto map = open_map();
    auto player = make_fighter();
    player.press_attack();
    player.step(kDt, {}, false, map);
    run_for(player, 6, {}, false, map);  // 0.12초 (아직 판정 중)
    player.press_attack();
    int started_at = -1;
    for (int i = 0; i < 12 && started_at < 0; ++i) {
        player.step(kDt, {}, false, map);
        if (player.attack_started()) started_at = i;
    }
    ASSERT_GE(started_at, 0) << "buffered press never chained";
    EXPECT_EQ(player.attack().combo_index(), 1);
    EXPECT_NEAR(player.stamina().current(), 72.0f, 1e-3f);  // 14 x 2
}

// 공격 중 구르기는 취소 구간이 열려야 나가요 (메모장이 그때까지 기억해요).
TEST(PlayerMotion, DodgeCancelsAttackOnlyInWindow) {
    const auto map = open_map();
    auto player = make_fighter();
    player.press_attack();
    player.step(kDt, {}, false, map);
    run_for(player, 3, {}, false, map);  // 약 0.07초
    player.press_dodge();
    for (int i = 0; i < 6; ++i) {  // 0.17초가 되기 전까지는 못 굴러요
        EXPECT_FALSE(player.step(kDt, kRight, false, map)) << "step " << i;
        EXPECT_EQ(player.state(), CharacterState::Attacking);
    }
    bool dodged = false;
    for (int i = 0; i < 4 && !dodged; ++i) {
        dodged = player.step(kDt, kRight, false, map);
    }
    EXPECT_TRUE(dodged);
    EXPECT_EQ(player.state(), CharacterState::Dodging);
    EXPECT_FALSE(player.attack().busy());  // 공격은 취소됐어요
}

// 공격 중에는 다리가 멈추고(서서히), Shift를 눌러도 달리지 않아요. 스태미나도 달리기로 안 줄어요.
TEST(PlayerMotion, AttackingStopsLegs) {
    const auto map = open_map();
    auto player = make_fighter();
    run_for(player, 30, kRight, false, map);
    ASSERT_NEAR(player.mover().speed(), 110.0f, 0.5f);
    player.press_attack();
    run_for(player, 12, kRight, true, map);
    EXPECT_EQ(player.state(), CharacterState::Attacking);
    EXPECT_LT(player.mover().speed(), 1.0f);
    EXPECT_NEAR(player.stamina().current(), 86.0f, 1e-3f);
}

// 공격 방향은 누른 방향. 히트박스는 판정 구간에만 있고, 그 방향 앞에 생겨요.
TEST(PlayerMotion, HitboxFacesInputAndOnlyWhileActive) {
    const auto map = open_map();
    auto player = make_fighter();  // 오른쪽을 보고 시작
    player.press_attack();
    player.step(kDt, {-1.0f, 0.0f}, false, map);
    EXPECT_EQ(player.mover().facing(), wind::world::Direction::Left);
    EXPECT_FALSE(player.attack_hitbox().has_value());  // 준비 중
    run_for(player, 5, {}, false, map);                 // 0.1초: 판정 중
    const auto box = player.attack_hitbox();
    ASSERT_TRUE(box.has_value());
    EXPECT_LT(box->right(), player.mover().feet().x);   // 왼쪽 앞
    EXPECT_TRUE(player.register_hit(1));
    EXPECT_FALSE(player.register_hit(1));                // 같은 휘두르기에 두 번은 안 맞아요
    run_for(player, 10, {}, false, map);
    EXPECT_FALSE(player.attack_hitbox().has_value());   // 후딜
}

// 무기가 없으면 공격 버튼을 눌러도 아무 일 없고, 스태미나도 그대로예요.
TEST(PlayerMotion, BareHandsDoNothing) {
    const auto map = open_map();
    auto player = make_player();
    player.press_attack();
    player.step(kDt, {}, false, map);
    EXPECT_FALSE(player.attack_started());
    EXPECT_FLOAT_EQ(player.stamina().current(), 100.0f);
}

}  // namespace
