// 이 파일은 무엇을 하나요?
// 몬스터 한 마리의 대기·배회(Monster)를 숙제 검사(테스트)해요. (11a)
// - 처음에는 제자리에서 쉬고, 쉬는 시간이 지나면 걷기 시작해요
// - 오래 돌아다녀도 제자리 반지름 밖으로 나가지 않아요
// - 같은 씨앗이면 같은 길, 다른 씨앗이면 다른 길로 걸어요 (재현)
// - 벽에 둘러싸여 있어도 벽 속에 들어가지 않고, 막히면 포기하고 다시 쉬어요
// - 시간 0(히트스톱)에는 아무것도 바뀌지 않아요
// - 11b: 감지 거리 안에서 알아채고(멈춤) → 쫓고 → 공격 거리에서 예고(방향 고정) → 40픽셀 덮치기 → 후딜
// - 11b: 주인공이 멀리 도망가거나 제자리에서 너무 멀어지면 집으로 가고, 영역 안이면 다시 알아채요
// - 11b: 같은 씨앗 + 같은 주인공 움직임이면 같은 결과 (재현)
// - 11d: 맞으면 HP가 줄고 움찔(예고는 취소, 덮치기는 계속)·넉백(벽 미끄러짐), HP 0이면 쓰러져 0.5초 뒤 사라져요
#include "game/monsters/monster.hpp"

#include "game/combat/combat_random.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace {

using wind::game::CombatRandom;
using wind::game::Monster;
using wind::game::MonsterState;

constexpr float kDt = 1.0f / 60.0f;
constexpr float kTile = 32.0f;

// 20x20칸 모두 지나갈 수 있는 맵
wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

// 테스트용 슬라임: 반지름 40, 걸음 20, 쉬기 0.5~1초
wind::combat::MonsterData slime_data() {
    wind::combat::MonsterData data;
    data.wander = {40.0f, 20.0f, 0.5f, 1.0f};
    data.body_width = 12.0f;
    data.body_height = 6.0f;
    data.hurtbox = {-9.0f, -13.0f, 18.0f, 13.0f};
    return data;
}

// 맵 가운데 칸 (10, 10)의 발 위치
wind::engine::Vec2 center() { return {10.5f * kTile, 10.5f * kTile}; }

TEST(Monster, StartsIdleAtHomeThenWanders) {
    const auto map = open_map();
    CombatRandom random(1);
    Monster slime("s", slime_data(), center());
    EXPECT_EQ(slime.state(), MonsterState::Idle);
    EXPECT_EQ(slime.feet(), center());
    // 처음 쉬는 시간(최소 0.5초)이 지나기 전에는 쉬어요
    for (int i = 0; i < 29; ++i) slime.step(kDt, map, random);
    EXPECT_EQ(slime.state(), MonsterState::Idle);
    // 0.5초를 조금 넘기면 걷기 시작해요
    for (int i = 0; i < 2; ++i) slime.step(kDt, map, random);
    EXPECT_EQ(slime.state(), MonsterState::Wander);
}

TEST(Monster, StaysInsideWanderRadius) {
    const auto map = open_map();
    CombatRandom random(7);
    Monster slime("s", slime_data(), center());
    bool walked = false;
    bool rested_again = false;
    for (int i = 0; i < 60 * 60; ++i) {  // 1분 동안
        slime.step(kDt, map, random);
        if (slime.state() == MonsterState::Wander) walked = true;
        if (walked && slime.state() == MonsterState::Idle) rested_again = true;  // 도착하면 다시 쉬어요
        // 반지름 40 안 (소수 오차만 봐줘요)
        ASSERT_LE(wind::engine::length(slime.feet() - slime.home()), 40.0f + 0.01f) << "step " << i;
    }
    EXPECT_TRUE(walked);
    EXPECT_TRUE(rested_again);
    EXPECT_GT(slime.distance_moved(), 40.0f);  // 여러 번 걸었어요
}

TEST(Monster, SameSeedSameWalk) {
    const auto map = open_map();
    // 씨앗으로 10초 걷게 한 뒤의 발 위치
    const auto walk = [&map](std::uint64_t seed) {
        CombatRandom random(seed);
        Monster slime("s", slime_data(), center());
        for (int i = 0; i < 600; ++i) slime.step(kDt, map, random);
        return slime.feet();
    };
    EXPECT_EQ(walk(2026), walk(2026));  // 같은 씨앗: 소수점까지 같아요
    EXPECT_NE(walk(2026), walk(2027));  // 다른 씨앗: 다른 곳
}

TEST(Monster, NeverEntersWallsAndGivesUpWhenStuck) {
    // 가운데 한 칸만 지나갈 수 있고 둘레가 모두 벽: 어디를 골라도 금방 막혀요
    auto map = open_map();
    map.walkable.assign(400, 0);
    map.walkable[static_cast<std::size_t>(map.index_of({10, 10}))] = 1;
    CombatRandom random(3);
    auto data = slime_data();
    data.wander.radius = 30.0f;  // 칸 밖 점도 고를 수 있게
    Monster slime("s", data, center());
    int idle_after_walk = 0;  // 걷다가 다시 쉬게 된 횟수
    MonsterState last = slime.state();
    for (int i = 0; i < 60 * 30; ++i) {  // 30초
        slime.step(kDt, map, random);
        ASSERT_FALSE(wind::game::box_blocked(map, wind::game::body_box(slime.feet(), slime.body())))
            << "step " << i;  // 발 상자는 절대 벽 속에 있지 않아요
        if (last == MonsterState::Wander && slime.state() == MonsterState::Idle) ++idle_after_walk;
        last = slime.state();
    }
    EXPECT_GT(idle_after_walk, 0);  // 막혀도 영원히 걷지 않고 쉬어요
}

TEST(Monster, ZeroTimeChangesNothing) {
    const auto map = open_map();
    CombatRandom random(5);
    Monster slime("s", slime_data(), center());
    for (int i = 0; i < 600 && slime.state() != MonsterState::Wander; ++i) {
        slime.step(kDt, map, random);  // 걷기 시작할 때까지
    }
    slime.step(kDt, map, random);      // 한 걸음 걸어서 전·지금 위치가 달라지게
    ASSERT_EQ(slime.state(), MonsterState::Wander);
    const auto before = slime.feet();
    const float moved = slime.distance_moved();
    for (int i = 0; i < 30; ++i) slime.step(0.0f, map, random);  // 히트스톱
    EXPECT_EQ(slime.feet(), before);
    EXPECT_FLOAT_EQ(slime.distance_moved(), moved);
    EXPECT_EQ(slime.interpolated_feet(0.0f), before);  // 그림도 멈춰 있어요
}

TEST(Monster, HurtboxFollowsFeet) {
    Monster slime("s", slime_data(), {100.0f, 200.0f});
    const auto box = slime.hurtbox();
    EXPECT_FLOAT_EQ(box.x, 91.0f);   // 100 - 9
    EXPECT_FLOAT_EQ(box.y, 187.0f);  // 200 - 13
    EXPECT_FLOAT_EQ(box.w, 18.0f);
    EXPECT_FLOAT_EQ(box.h, 13.0f);
}

// ---------------------------------------------------------------- 11b: 알아채기·쫓기·예고·덮치기·복귀 ----

// 40x40칸(1280픽셀) 모두 지나갈 수 있는 넓은 맵 (도망·복귀 테스트용)
wind::world::MapData wide_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 40;
    map.height = 40;
    map.walkable.assign(1600, 1);
    return map;
}

// 덤비는 슬라임: 감지 120, 공격 거리 40, 추적 한계 260, 알아채기 0.4초, 쫓기 40px/s,
// 덮치기 예고 0.6초 → 판정 0.25초 동안 40픽셀 → 후딜 0.7초 (content/attacks/slime_hop.json과 같은 값)
wind::combat::MonsterData fighter_data() {
    auto data = slime_data();
    data.move_speed = 40.0f;
    data.detect_range = 120.0f;
    data.attack_range = 40.0f;
    data.leash_range = 260.0f;
    data.notice_seconds = 0.4f;
    wind::combat::AttackData hop;
    hop.id = "hop";
    hop.windup = 0.6f;
    hop.active = 0.25f;
    hop.recovery = 0.7f;
    hop.lunge = 40.0f;
    hop.hitbox = {6.0f, -6.0f, 18.0f, 14.0f};
    data.attacks.push_back(hop);
    return data;
}

// 넓은 맵 가운데 (20, 20)칸의 발 위치
wind::engine::Vec2 wide_center() { return {20.5f * kTile, 20.5f * kTile}; }

// 상태가 want가 될 때까지(최대 limit 박자) 걸어요. 그 상태가 되면 true
bool run_until(Monster& slime, MonsterState want, const wind::world::MapData& map, CombatRandom& random,
               wind::engine::Vec2 hero, int limit = 1200) {
    for (int i = 0; i < limit && slime.state() != want; ++i) slime.step(kDt, map, random, hero);
    return slime.state() == want;
}

TEST(MonsterAi, NoticesOnlyInsideDetectRange) {
    const auto map = wide_map();
    CombatRandom random(1);
    Monster far_slime("s", fighter_data(), wide_center());
    far_slime.step(kDt, map, random, wide_center() + wind::engine::Vec2{130.0f, 0.0f});  // 감지 120 밖
    EXPECT_EQ(far_slime.state(), MonsterState::Idle);

    Monster slime("s", fighter_data(), wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{100.0f, 0.0f};  // 감지 120 안
    slime.step(kDt, map, random, hero);
    EXPECT_EQ(slime.state(), MonsterState::Notice);
    for (int i = 0; i < 22; ++i) slime.step(kDt, map, random, hero);  // 0.4초가 되기 전에는 멈춰서 느낌표
    EXPECT_EQ(slime.state(), MonsterState::Notice);
    EXPECT_EQ(slime.feet(), wide_center());                             // 알아채는 동안 움직이지 않아요
    for (int i = 0; i < 3; ++i) slime.step(kDt, map, random, hero);
    EXPECT_EQ(slime.state(), MonsterState::Chase);
}

TEST(MonsterAi, ChasesThenTelegraphsThenLunges) {
    const auto map = wide_map();
    CombatRandom random(1);
    Monster slime("s", fighter_data(), wide_center());
    auto hero = wide_center() + wind::engine::Vec2{100.0f, 0.0f};
    ASSERT_TRUE(run_until(slime, MonsterState::Windup, map, random, hero));
    // 공격 거리 안에서 멈춰 예고해요. 덮칠 길은 주인공 쪽(오른쪽)으로 40픽셀
    EXPECT_LE(wind::engine::length(hero - slime.feet()), 40.0f + 0.01f);
    ASSERT_TRUE(slime.lunge_end().has_value());
    EXPECT_NEAR(slime.lunge_end()->x - slime.feet().x, 40.0f, 0.01f);
    EXPECT_NEAR(slime.lunge_end()->y, slime.feet().y, 0.01f);
    EXPECT_FALSE(slime.attack_hitbox().has_value());  // 예고 중에는 판정이 없어요 (피할 시간)
    // 예고 중에 주인공이 위로 피해도 덮칠 방향은 바뀌지 않아요 (예고를 믿고 피할 수 있게)
    hero = slime.feet() + wind::engine::Vec2{0.0f, -60.0f};
    const auto windup_feet = slime.feet();
    ASSERT_TRUE(run_until(slime, MonsterState::Attack, map, random, hero, 60));
    EXPECT_EQ(slime.feet(), windup_feet);              // 예고 동안 제자리
    EXPECT_FLOAT_EQ(slime.attack_direction().x, 1.0f);
    // 덮치는 동안만 판정 상자가 있고, 판정이 끝나면 40픽셀 앞에 있어요
    bool had_hitbox = false;
    for (int i = 0; i < 60 && slime.state() == MonsterState::Attack; ++i) {
        had_hitbox = had_hitbox || slime.attack_hitbox().has_value();
        slime.step(kDt, map, random, hero);
    }
    EXPECT_TRUE(had_hitbox);
    EXPECT_EQ(slime.state(), MonsterState::Recover);
    EXPECT_FALSE(slime.attack_hitbox().has_value());
    EXPECT_NEAR(slime.feet().x - windup_feet.x, 40.0f, 0.01f);
    EXPECT_NEAR(slime.feet().y, windup_feet.y, 0.01f);
}

TEST(MonsterAi, GivesUpWhenHeroRunsFarAndWalksHome) {
    const auto map = wide_map();
    CombatRandom random(2);
    Monster slime("s", fighter_data(), wide_center());
    const auto near = wide_center() + wind::engine::Vec2{100.0f, 0.0f};
    ASSERT_TRUE(run_until(slime, MonsterState::Chase, map, random, near));
    for (int i = 0; i < 30; ++i) slime.step(kDt, map, random, near);  // 조금 쫓아가서 제자리를 벗어나요
    const auto far = slime.feet() + wind::engine::Vec2{0.0f, 300.0f};  // 주인공이 300픽셀 밖으로 도망
    slime.step(kDt, map, random, far);
    EXPECT_EQ(slime.state(), MonsterState::Return);
    ASSERT_TRUE(run_until(slime, MonsterState::Idle, map, random, far));  // 집에 오면 다시 쉬어요
    EXPECT_LE(wind::engine::length(slime.feet() - slime.home()), 1.0f);
}

TEST(MonsterAi, GivesUpWhenDraggedTooFarFromHome) {
    const auto map = wide_map();
    CombatRandom random(3);
    Monster slime("s", fighter_data(), wide_center());
    // 주인공이 슬라임보다 조금 느리게(35px/s) 오른쪽으로 계속 도망쳐요: 공격 거리에는 안 닿고 계속 쫓겨요
    auto hero = wide_center() + wind::engine::Vec2{100.0f, 0.0f};
    bool returned = false;
    for (int i = 0; i < 60 * 20 && !returned; ++i) {
        if (slime.state() == MonsterState::Chase) hero.x += 35.0f * kDt;
        slime.step(kDt, map, random, hero);
        returned = slime.state() == MonsterState::Return;
    }
    ASSERT_TRUE(returned);
    EXPECT_GT(wind::engine::length(slime.feet() - slime.home()), 260.0f);  // 추적 한계를 넘어서 포기
    // 주인공이 감지 거리 안이어도 영역 밖이면 다시 알아채지 않고 집에 가요 (경계에서 왔다 갔다 하지 않게)
    for (int i = 0; i < 60; ++i) {
        slime.step(kDt, map, random, slime.feet() + wind::engine::Vec2{60.0f, 0.0f});
        ASSERT_EQ(slime.state(), MonsterState::Return) << "step " << i;
    }
}

TEST(MonsterAi, RenoticesWhileReturningInsideTerritory) {
    const auto map = wide_map();
    CombatRandom random(4);
    Monster slime("s", fighter_data(), wide_center());
    const auto near = wide_center() + wind::engine::Vec2{100.0f, 0.0f};
    ASSERT_TRUE(run_until(slime, MonsterState::Chase, map, random, near));
    slime.step(kDt, map, random, wide_center() + wind::engine::Vec2{0.0f, 500.0f});  // 멀리 도망 → 복귀
    ASSERT_EQ(slime.state(), MonsterState::Return);
    slime.step(kDt, map, random, slime.feet() + wind::engine::Vec2{-50.0f, 0.0f});  // 영역 안으로 다시 옴
    EXPECT_EQ(slime.state(), MonsterState::Notice);
}

TEST(MonsterAi, HitstopFreezesTelegraph) {
    const auto map = wide_map();
    CombatRandom random(5);
    Monster slime("s", fighter_data(), wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{100.0f, 0.0f};
    ASSERT_TRUE(run_until(slime, MonsterState::Windup, map, random, hero));
    const float before = slime.state_seconds();
    for (int i = 0; i < 120; ++i) slime.step(0.0f, map, random, hero);  // 2초 동안 히트스톱
    EXPECT_EQ(slime.state(), MonsterState::Windup);
    EXPECT_FLOAT_EQ(slime.state_seconds(), before);
}

// 같은 씨앗 + 같은 주인공 움직임(입력 기록)이면 소수점까지 같은 결과 (명세 17.2)
TEST(MonsterAi, SameSeedSameHeroSameResult) {
    const auto map = wide_map();
    const auto replay = [&map](std::uint64_t seed) {
        CombatRandom random(seed);
        Monster slime("s", fighter_data(), wide_center());
        std::vector<MonsterState> states;
        for (int i = 0; i < 60 * 30; ++i) {  // 30초: 주인공이 다가왔다 멀어졌다 해요
            const float t = static_cast<float>(i) * kDt;
            const float reach = 30.0f + 200.0f * (0.5f + 0.5f * std::sin(t * 0.7f));
            const wind::engine::Vec2 hero = wide_center() + wind::engine::Vec2{reach, 20.0f * std::sin(t * 2.3f)};
            slime.step(kDt, map, random, hero);
            states.push_back(slime.state());
        }
        return std::make_pair(slime.feet(), states);
    };
    const auto a = replay(2026);
    const auto b = replay(2026);
    EXPECT_EQ(a.first, b.first);
    EXPECT_EQ(a.second, b.second);
    // 이 기록에서는 예고와 덮치기가 실제로 일어나요 (재현 테스트가 싸움을 다루는지 확인)
    EXPECT_NE(std::ranges::find(a.second, MonsterState::Attack), a.second.end());
}


// ---------------------------------------------------------------- 11d: 맞기·쓰러지기 ----

TEST(MonsterHit, DamageStaggersThenChasesAndDeathRemovesIt) {
    const auto map = wide_map();
    CombatRandom random(1);
    auto data = fighter_data();
    data.max_hp = 30;
    data.stagger_seconds = 0.25f;
    Monster slime("s", data, wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{200.0f, 0.0f};  // 감지 120 밖이지만 추적 한계 260 안
    EXPECT_EQ(slime.take_damage(14), 14);
    EXPECT_EQ(slime.hp(), 16);
    slime.push({1.0f, 0.0f}, 90.0f);
    EXPECT_EQ(slime.state(), MonsterState::Hurt);  // 쉬다가 맞으면 움찔
    for (int i = 0; i < 14; ++i) slime.step(kDt, map, random, hero);
    EXPECT_EQ(slime.state(), MonsterState::Hurt);
    EXPECT_GT(slime.feet().x, wide_center().x);     // 맞은 쪽으로 밀렸어요
    for (int i = 0; i < 2; ++i) slime.step(kDt, map, random, hero);
    EXPECT_EQ(slime.state(), MonsterState::Chase);  // 움찔이 끝나면 쫓아와요 (멀리서 맞아도 알아채요)

    EXPECT_EQ(slime.take_damage(100), 16);          // 남은 HP만큼만 깎여요
    EXPECT_TRUE(slime.defeated());
    EXPECT_FALSE(slime.can_be_hit());
    EXPECT_EQ(slime.take_damage(10), 0);            // 쓰러진 뒤에는 안 맞아요
    slime.push({1.0f, 0.0f}, 90.0f);
    EXPECT_EQ(slime.state(), MonsterState::Dead);   // 밀어도 쓰러진 채
    for (int i = 0; i < 29; ++i) slime.step(kDt, map, random, hero);
    EXPECT_FALSE(slime.gone());                     // 0.5초 동안 쓰러지는 연출
    for (int i = 0; i < 2; ++i) slime.step(kDt, map, random, hero);
    EXPECT_TRUE(slime.gone());
    EXPECT_EQ(slime.state(), MonsterState::Dead);   // 사라져도 다시 일어나지 않아요
}

TEST(MonsterHit, HitDuringWindupCancelsTheAttack) {
    const auto map = wide_map();
    CombatRandom random(2);
    Monster slime("s", fighter_data(), wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{30.0f, 0.0f};
    ASSERT_TRUE(run_until(slime, MonsterState::Windup, map, random, hero));
    slime.push({1.0f, 0.0f}, 60.0f);  // 예고 중에 맞음
    EXPECT_EQ(slime.state(), MonsterState::Hurt);
    bool attacked = false;            // 남은 예고 시간 동안 덮치지 않아요 (공격 취소)
    for (int i = 0; i < 20; ++i) {
        slime.step(kDt, map, random, hero);
        attacked = attacked || slime.state() == MonsterState::Attack;
    }
    EXPECT_FALSE(attacked);
}

TEST(MonsterHit, HitDuringLungeKeepsLunging) {
    const auto map = wide_map();
    CombatRandom random(3);
    Monster slime("s", fighter_data(), wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{30.0f, 0.0f};
    ASSERT_TRUE(run_until(slime, MonsterState::Attack, map, random, hero));
    const float before_y = slime.feet().y;
    slime.push({0.0f, 1.0f}, 160.0f);  // 덮치는 중에 아래쪽으로 맞음
    EXPECT_EQ(slime.state(), MonsterState::Attack);  // 덮치기는 계속 (개발자와 정함)
    for (int i = 0; i < 5; ++i) slime.step(kDt, map, random, hero);
    EXPECT_EQ(slime.state(), MonsterState::Attack);
    EXPECT_GT(slime.feet().y, before_y);             // 그래도 밀리기는 해요
}

TEST(MonsterHit, KnockbackSlidesAlongWall) {
    // 오른쪽 21번째 칸 줄이 벽: 오른쪽 아래로 밀리면 벽에 막히고 아래로만 미끄러져요
    auto map = wide_map();
    for (int y = 0; y < map.height; ++y) map.walkable[static_cast<std::size_t>(map.index_of({21, y}))] = 0;
    CombatRandom random(4);
    auto data = fighter_data();
    data.stagger_seconds = 0.25f;
    const wind::engine::Vec2 start{21.0f * kTile - 8.0f, 20.5f * kTile};  // 벽 왼쪽 8픽셀 (발 상자 반폭 6)
    Monster slime("s", data, start);
    slime.push({0.70710678f, 0.70710678f}, 160.0f);
    for (int i = 0; i < 15; ++i) slime.step(kDt, map, random, std::nullopt);  // 움찔 0.25초 (끝나면 집으로 걸어가요)
    EXPECT_LE(slime.feet().x + 6.0f, 21.0f * kTile);                  // 벽 속으로 안 들어가요
    EXPECT_NEAR(slime.feet().y - start.y, 20.0f * 0.70710678f, 0.5f);  // 160 × 0.25 ÷ 2 = 20픽셀 중 아래쪽 몫
}

}  // namespace

// ---------------------------------------------------------------- 13c: 개구리 (물러나기·침) ----

namespace {

// 침 뱉는 개구리: 공격 거리 130, 물러나는 거리 70, 침 (빠르기 170, 거리 200)
wind::combat::MonsterData frog_data() {
    auto data = fighter_data();
    data.detect_range = 160.0f;
    data.attack_range = 130.0f;
    data.keep_away = 70.0f;
    data.leash_range = 300.0f;
    data.attacks.front().lunge = 0.0f;
    data.attacks.front().projectile = wind::combat::ProjectileData{170.0f, 200.0f, 8.0f, 8.0f};
    return data;
}

}  // namespace

TEST(MonsterAi, FrogBacksAwayWhenHeroIsClose) {
    const auto map = wide_map();
    CombatRandom random(1);
    Monster frog("f", frog_data(), wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{40.0f, 0.0f};  // 70보다 가까워요
    ASSERT_TRUE(run_until(frog, MonsterState::Chase, map, random, hero));
    const float before = frog.feet().x;
    for (int i = 0; i < 30; ++i) frog.step(kDt, map, random, hero);
    EXPECT_LT(frog.feet().x, before);  // 주인공 반대쪽(왼쪽)으로 물러났어요
}

TEST(MonsterAi, FrogShootsOncePerAttackWithoutContactHitbox) {
    const auto map = wide_map();
    CombatRandom random(1);
    Monster frog("f", frog_data(), wide_center());
    const auto hero = wide_center() + wind::engine::Vec2{100.0f, 0.0f};  // 70~130 사이: 물러나지 않고 예고
    ASSERT_TRUE(run_until(frog, MonsterState::Windup, map, random, hero));
    ASSERT_TRUE(frog.lunge_end().has_value());
    EXPECT_NEAR(frog.lunge_end()->x - frog.feet().x, 200.0f, 0.5f);  // 예고 길은 침이 날아갈 거리만큼
    ASSERT_TRUE(run_until(frog, MonsterState::Attack, map, random, hero));
    EXPECT_FALSE(frog.attack_hitbox().has_value());  // 몸으로는 안 부딪혀요
    EXPECT_TRUE(frog.take_shot());                   // 한 방
    EXPECT_FALSE(frog.take_shot());                  // 두 번은 안 쏴요
}
