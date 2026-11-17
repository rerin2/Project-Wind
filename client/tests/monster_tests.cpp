// 이 파일은 무엇을 하나요?
// 몬스터 한 마리의 대기·배회(Monster)를 숙제 검사(테스트)해요. (11a)
// - 처음에는 제자리에서 쉬고, 쉬는 시간이 지나면 걷기 시작해요
// - 오래 돌아다녀도 제자리 반지름 밖으로 나가지 않아요
// - 같은 씨앗이면 같은 길, 다른 씨앗이면 다른 길로 걸어요 (재현)
// - 벽에 둘러싸여 있어도 벽 속에 들어가지 않고, 막히면 포기하고 다시 쉬어요
// - 시간 0(히트스톱)에는 아무것도 바뀌지 않아요
#include "game/monsters/monster.hpp"

#include "game/combat/combat_random.hpp"

#include <gtest/gtest.h>

#include <cstdint>

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

}  // namespace
