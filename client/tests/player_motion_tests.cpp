// 이 파일은 무엇을 하나요?
// 주인공 몸 움직임(PlayerMotion)이 스태미나 규칙대로 달리고 구르는지 숙제 검사해요. (8단계 완료 조건)
// - Shift를 누르고 움직이면 1.5배 빠르게 달리고, 1초에 25씩 스태미나를 써요
// - 제자리에서 Shift만 누르면 스태미나를 안 써요
// - 구르기는 25를 쓰고, 25가 안 되면 구르지 않아요
// - 다 쓰면 지침: 25까지 차야 다시 달리고 구를 수 있어요
// - 11c: 맞으면 HP가 줄고 경직(행동 막힘)·넉백(벽 미끄러짐)·무적 0.6초, 구르기 처음 0.16초는 안 맞아요
// - 12b: HP 0이면 쓰러짐(dead). 버튼·이동·스태미나·MP가 멈추고 더 맞지 않아요. 마지막 넉백은 끝까지 밀려요
#include "game/entities/player_motion.hpp"

#include "wind/combat/weapon.hpp"

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

// MP는 주인공과 함께 최대 50으로 시작하고, 달리기·공격에는 안 줄어요 (9d).
TEST(PlayerMotion, StartsWithFullManaThatOnlySpellsUse) {
    const auto map = open_map();
    auto player = make_player();
    EXPECT_FLOAT_EQ(player.mana().current(), 50.0f);
    EXPECT_FLOAT_EQ(player.mana().max(), 50.0f);
    run_for(player, 60, kRight, true, map);  // 1초 달리기: 스태미나만 줄어요
    EXPECT_FLOAT_EQ(player.mana().current(), 50.0f);
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

// ---- 9e: 마술 시전 (명세 8.6, 9.2) ----

PlayerMotion make_caster() {
    auto player = make_fighter();
    wind::combat::Spell spell;
    spell.id = "star_shard";
    spell.mp_cost = 6;
    spell.cast_seconds = 0.25f;
    spell.projectile = wind::combat::ProjectileData{360.0f, 240.0f, 6.0f, 6.0f};
    player.set_spell(spell);
    return player;
}

// 누르면 MP 6을 먼저 내고, 0.25초(15박자) 시전한 뒤 한 박자만 "나갔다"를 알려 줘요.
TEST(PlayerMotion, CastSpendsManaThenReleasesOnce) {
    const auto map = open_map();
    auto player = make_caster();
    player.press_cast();
    player.step(kDt, {}, false, map);
    EXPECT_EQ(player.state(), CharacterState::Casting);
    EXPECT_NEAR(player.mana().current(), 44.0f, 0.1f);  // 50 - 6 (+ 한 박자 회복)
    int released = 0;
    int released_at = -1;
    for (int i = 2; i <= 20; ++i) {
        player.step(kDt, {}, false, map);
        if (player.spell_released()) {
            ++released;
            released_at = i;
        }
    }
    EXPECT_EQ(released, 1);
    EXPECT_EQ(released_at, 15);
    EXPECT_NE(player.state(), CharacterState::Casting);
}

// MP가 모자라면 마술이 나가지 않고, MP도 그대로이고, "모자람" 횟수만 올라요. (9단계 완료 조건)
TEST(PlayerMotion, NotEnoughManaDoesNotCast) {
    const auto map = open_map();
    auto player = make_caster();
    // 9번 쏘기: 6 x 9 = 54를 쓰고, 그동안 약 4.8 회복해서 1 정도 남아요
    for (int shot = 0; shot < 9; ++shot) {
        player.press_cast();
        run_for(player, 16, {}, false, map);
    }
    ASSERT_LT(player.mana().current(), 6.0f);
    const float before = player.mana().current();
    player.press_cast();
    player.step(kDt, {}, false, map);
    EXPECT_NE(player.state(), CharacterState::Casting);
    EXPECT_NEAR(player.mana().current(), before + 2.0f / 60.0f, 1e-3f);  // 회복만 됐어요
    EXPECT_EQ(player.mana_denied_count(), 1u);
    run_for(player, 20, {}, false, map);  // 메모는 지워져서 나중에 저절로 나가지도 않아요
    EXPECT_EQ(player.mana_denied_count(), 1u);
    EXPECT_NE(player.state(), CharacterState::Casting);
}

// 시전 중에는 천천히 걷고(0.4배 = 44 px/s), 달리지 않고, 바라보는 방향은 시전 방향 그대로예요.
TEST(PlayerMotion, CastingWalksSlowlyAndKeepsFacing) {
    const auto map = open_map();
    auto player = make_caster();  // 오른쪽을 보고 시작
    player.press_cast();
    player.step(kDt, {}, false, map);
    run_for(player, 12, {0.0f, 1.0f}, true, map);  // 아래로 Shift 걷기
    EXPECT_EQ(player.state(), CharacterState::Casting);
    EXPECT_NEAR(player.mover().speed(), 44.0f, 1.0f);
    EXPECT_EQ(player.mover().facing(), wind::world::Direction::Right);
    EXPECT_FLOAT_EQ(player.stamina().current(), 100.0f);  // 달리지 않았어요
}

// 마술 방향은 누른 방향이에요.
TEST(PlayerMotion, CastFacesInput) {
    const auto map = open_map();
    auto player = make_caster();
    player.press_cast();
    player.step(kDt, {0.0f, -1.0f}, false, map);
    EXPECT_EQ(player.cast_facing(), wind::world::Direction::Up);
}

// 시전 중에는 구르기도 공격도 안 돼요. 끝나면 메모해 둔 구르기가 나갈 수 있어요.
TEST(PlayerMotion, CastingBlocksDodgeAndAttack) {
    const auto map = open_map();
    auto player = make_caster();
    player.press_cast();
    player.step(kDt, {}, false, map);
    player.press_dodge();
    player.press_attack();
    EXPECT_FALSE(player.step(kDt, kRight, false, map));
    EXPECT_FALSE(player.attack_started());
    EXPECT_EQ(player.state(), CharacterState::Casting);
}

// 공격 중에는 마술이 안 나가요 (상태 표). MP도 그대로예요.
TEST(PlayerMotion, AttackingBlocksCast) {
    const auto map = open_map();
    auto player = make_caster();
    player.press_attack();
    player.step(kDt, {}, false, map);
    player.press_cast();
    player.step(kDt, {}, false, map);
    EXPECT_EQ(player.state(), CharacterState::Attacking);
    EXPECT_FLOAT_EQ(player.mana().current(), 50.0f);
}

// 마술이 없으면 눌러도 아무 일 없어요.
TEST(PlayerMotion, NoSpellDoesNothing) {
    const auto map = open_map();
    auto player = make_player();
    player.press_cast();
    player.step(kDt, {}, false, map);
    EXPECT_NE(player.state(), CharacterState::Casting);
    EXPECT_FLOAT_EQ(player.mana().current(), 50.0f);
    EXPECT_EQ(player.mana_denied_count(), 0u);
}


// ---------------------------------------------------------------- 11c: 맞기 ----

TEST(PlayerMotion, HitStaggersAndKnocksBack) {
    const auto map = open_map();
    auto player = make_player();
    EXPECT_TRUE(player.can_be_hit());
    EXPECT_EQ(player.take_hit(4, kRight, 160.0f), 4);
    EXPECT_EQ(player.health().current(), 96);
    EXPECT_EQ(player.state(), CharacterState::Hit);
    EXPECT_FALSE(player.can_be_hit());  // 맞은 직후 무적
    // 경직 0.3초 동안은 왼쪽 키도 구르기도 안 먹고, 오른쪽으로 24픽셀 밀려나요
    player.press_dodge();
    run_for(player, 18, Vec2{-1.0f, 0.0f}, false, map);
    EXPECT_NE(player.mover().state(), Mover::State::Dodging);
    EXPECT_NEAR(player.mover().feet().x, 124.0f, 0.5f);
    EXPECT_NEAR(player.mover().feet().y, 300.0f, 0.01f);
    // 경직이 끝나면 다시 걸어요
    run_for(player, 2, Vec2{}, false, map);
    EXPECT_NE(player.state(), CharacterState::Hit);
    const float before = player.mover().feet().x;
    run_for(player, 20, Vec2{-1.0f, 0.0f}, false, map);
    EXPECT_LT(player.mover().feet().x, before);
}

TEST(PlayerMotion, KnockbackSlidesAlongWall) {
    // 오른쪽 4번째 칸 줄(x 128~160)이 벽. 주인공 발 상자 오른쪽 끝은 125
    auto map = open_map();
    for (int y = 0; y < map.height; ++y) map.walkable[static_cast<std::size_t>(map.index_of({4, y}))] = 0;
    PlayerMotion player({118.0f, 300.0f}, wind::world::Direction::Right, MovementTuning{}, StaminaTuning{}, 0.15);
    const Vec2 diagonal{0.70710678f, 0.70710678f};  // 오른쪽 아래로 밀려요
    (void)player.take_hit(4, diagonal, 160.0f);
    run_for(player, 20, Vec2{}, false, map);
    EXPECT_LE(player.mover().feet().x + 7.0f, 128.0f);    // 벽 속으로 안 들어가요
    EXPECT_NEAR(player.mover().feet().y, 300.0f + 24.0f * diagonal.y, 0.5f);  // 아래로는 끝까지 미끄러져요
}

TEST(PlayerMotion, InvulnerabilityEndsAfterPointSixSeconds) {
    const auto map = open_map();
    auto player = make_player();
    (void)player.take_hit(4, kRight, 0.0f);
    run_for(player, 35, Vec2{}, false, map);  // 0.583초
    EXPECT_FALSE(player.can_be_hit());
    run_for(player, 2, Vec2{}, false, map);   // 0.617초
    EXPECT_TRUE(player.can_be_hit());
}

TEST(PlayerMotion, DodgeStartIsInvulnerable) {
    const auto map = open_map();
    auto player = make_player();
    player.press_dodge();
    player.step(kDt, kRight, false, map);
    ASSERT_EQ(player.mover().state(), Mover::State::Dodging);
    EXPECT_FALSE(player.can_be_hit());  // 구르기 처음 0.16초는 안 맞아요 (11c에서 연결)
    run_for(player, 12, kRight, false, map);  // 0.217초 (구르기 무적 0.16초가 지났어요)
    EXPECT_TRUE(player.can_be_hit());
}

TEST(PlayerMotion, HitCancelsAttack) {
    const auto map = open_map();
    wind::combat::Weapon weapon;
    wind::combat::AttackData swing;
    swing.id = "s";
    swing.windup = 0.1f;
    swing.active = 0.1f;
    swing.recovery = 0.2f;
    weapon.combo.push_back(swing);
    PlayerMotion player({100.0f, 300.0f}, wind::world::Direction::Right, MovementTuning{}, StaminaTuning{}, 0.15,
                        weapon);
    player.press_attack();
    player.step(kDt, Vec2{}, false, map);
    ASSERT_EQ(player.state(), CharacterState::Attacking);
    (void)player.take_hit(4, kRight, 160.0f);
    EXPECT_EQ(player.state(), CharacterState::Hit);   // 칼은 끊기고 경직
    EXPECT_FALSE(player.attack().busy());
}

// ---------------------------------------------------------------- 12b: 쓰러짐 ----

TEST(PlayerMotion, ZeroHpMeansDead) {
    auto player = make_player();
    EXPECT_FALSE(player.dead());
    EXPECT_EQ(player.take_hit(250, kRight, 0.0f), 100);  // 남은 100만 깎여요
    EXPECT_EQ(player.health().current(), 0);
    EXPECT_TRUE(player.dead());
    EXPECT_EQ(player.state(), CharacterState::Dead);  // 경직(hit)보다 쓰러짐이 먼저
}

TEST(PlayerMotion, DeadHeroCannotActMoveOrBeHit) {
    const auto map = open_map();
    wind::combat::Weapon weapon;
    wind::combat::AttackData swing;
    swing.id = "s";
    swing.windup = 0.1f;
    swing.active = 0.1f;
    swing.recovery = 0.2f;
    weapon.combo.push_back(swing);
    PlayerMotion player({100.0f, 300.0f}, wind::world::Direction::Right, MovementTuning{}, StaminaTuning{}, 0.15,
                        weapon);
    run_for(player, 30, kRight, true, map);         // 달려서 스태미나를 조금 써 둬요
    (void)player.take_hit(1000, kRight, 0.0f);      // 넉백 없이 쓰러짐
    ASSERT_TRUE(player.dead());
    run_for(player, 30, Vec2{}, false, map);        // 미끄러지던 몸이 멈출 시간
    const Vec2 lying = player.mover().feet();
    const float stamina = player.stamina().current();
    const float mana = player.mana().current();
    for (int i = 0; i < 60; ++i) {                  // 1초 동안 모든 버튼을 눌러요
        player.press_attack();
        player.press_dodge();
        player.press_cast();
        player.step(kDt, Vec2{-1.0f, 0.0f}, true, map);
        EXPECT_FALSE(player.attack_started());
        EXPECT_EQ(player.state(), CharacterState::Dead);
    }
    EXPECT_EQ(player.mover().feet().x, lying.x);    // 한 픽셀도 안 움직여요
    EXPECT_EQ(player.mover().feet().y, lying.y);
    EXPECT_FALSE(player.attack().busy());
    EXPECT_FLOAT_EQ(player.stamina().current(), stamina);  // 쓰러져 있으면 차지 않아요
    EXPECT_FLOAT_EQ(player.mana().current(), mana);
    EXPECT_FALSE(player.can_be_hit());                      // 무적이 끝나도 안 맞아요
    EXPECT_EQ(player.take_hit(10, kRight, 160.0f), 0);      // 더 맞지 않고, 밀리지도 않아요
    player.step(kDt, Vec2{}, false, map);
    EXPECT_EQ(player.mover().feet().x, lying.x);
}

// ---------------------------------------------------------------- 12c: 맵을 옮길 때 들고 가기 ----

TEST(PlayerMotion, VitalsCarryOverToANewBody) {
    const auto map = open_map();
    auto before = make_player();
    (void)before.take_hit(40, kRight, 0.0f);   // HP 60
    run_for(before, 30, kRight, true, map);    // 달려서 스태미나를 써요
    const auto carried = before.vitals();
    EXPECT_EQ(carried.hp, 60);
    EXPECT_LT(carried.stamina, 100.0f);

    auto after = make_player();                // 새 맵의 새 몸 (가득 찬 채 태어나요)
    after.restore(carried);
    EXPECT_EQ(after.health().current(), 60);   // 다시 차지 않아요
    EXPECT_FLOAT_EQ(after.stamina().current(), carried.stamina);
    EXPECT_FLOAT_EQ(after.mana().current(), carried.mana);
    EXPECT_FALSE(after.dead());
}

TEST(PlayerMotion, RestoreClampsBadValuesAndKeepsExhaustion) {
    auto player = make_player();
    player.restore({500, -10.0f, 10.0f, true});    // 최대보다 큰 HP, 음수 MP, 지친 스태미나 10
    EXPECT_EQ(player.health().current(), 100);
    EXPECT_FLOAT_EQ(player.mana().current(), 0.0f);
    EXPECT_FLOAT_EQ(player.stamina().current(), 10.0f);
    EXPECT_TRUE(player.stamina().exhausted());     // 25까지 차기 전이라 계속 지쳐 있어요
    player.restore({-5, 20.0f, 100.0f, true});     // 음수 HP는 0, 가득 찬 스태미나는 지침이 아니에요
    EXPECT_EQ(player.health().current(), 0);
    EXPECT_FALSE(player.stamina().exhausted());
}

TEST(PlayerMotion, FatalHitStillKnocksBack) {
    const auto map = open_map();
    auto player = make_player();
    (void)player.take_hit(1000, kRight, 160.0f);  // 마지막 한 방도 24픽셀 밀려나요 (넉백은 끝까지)
    run_for(player, 30, Vec2{}, false, map);
    EXPECT_NEAR(player.mover().feet().x, 124.0f, 0.5f);
}

}  // namespace

// ---------------------------------------------------------------- 13b: 쿠션에서 쉬기 ----

TEST(PlayerMotion, RestingBlocksActionsAndRefills) {
    const auto map = open_map();
    auto player = make_player();
    player.take_hit(40, {1.0f, 0.0f}, 0.0f);
    run_for(player, 30, {}, false, map);  // 경직이 끝나도록
    ASSERT_TRUE(player.begin_rest({110.0f, 300.0f}));
    EXPECT_EQ(player.health().current(), player.health().max());
    EXPECT_EQ(player.state(), CharacterState::Resting);
    player.press_attack();
    player.press_dodge();
    run_for(player, 30, {}, false, map);
    EXPECT_FALSE(player.attack().busy());                       // 쉬는 중에는 칼이 안 나가요
    EXPECT_EQ(player.state(), CharacterState::Resting);
    EXPECT_NEAR(player.mover().feet().x, 110.0f, 0.01f);        // 쿠션 자리로 옮겨 갔어요
}

TEST(PlayerMotion, CannotStartRestWhileDodging) {
    const auto map = open_map();
    auto player = make_player();
    player.press_dodge();
    player.step(kDt, kRight, false, map);
    ASSERT_EQ(player.state(), CharacterState::Dodging);
    EXPECT_FALSE(player.begin_rest({110.0f, 300.0f}));  // 구르는 중에는 못 앉아요 (상태 표)
}

TEST(PlayerMotion, SecondsSinceHitResetsOnHit) {
    const auto map = open_map();
    auto player = make_player();
    EXPECT_GE(player.seconds_since_hit(), 3.0f);  // 한 번도 안 맞았으면 아주 오래전
    player.take_hit(1, {1.0f, 0.0f}, 0.0f);
    EXPECT_FLOAT_EQ(player.seconds_since_hit(), 0.0f);
    run_for(player, 60, {}, false, map);
    EXPECT_NEAR(player.seconds_since_hit(), 1.0f, 0.01f);
}
