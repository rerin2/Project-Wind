// 이 파일은 무엇을 하나요?
// 캐릭터의 다리(Mover)가 규칙대로 움직이는지 숙제 검사(테스트)를 해요.
// - 서서히 빨라지고, 최고 속도를 넘지 않고, 대각선도 더 빠르지 않나
// - 키를 떼면 멈추나
// - 구르기는 빠르고, 짧고, 끝나면 쉬는 시간이 있나
// - 벽에 막히면 속도가 0이 되나
// - 화면용 중간 위치(보간)가 맞나
#include "game/entities/mover.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Vec2;
using wind::game::Mover;
using wind::game::MovementTuning;
using wind::world::Direction;

constexpr double kDt = 1.0 / 60.0;  // 한 박자

// 벽이 없는 넓은 20x20 맵
wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

// 박자를 n번 돌려요.
void run(Mover& mover, int steps, Vec2 input, const wind::world::MapData& map) {
    for (int i = 0; i < steps; ++i) {
        mover.step(kDt, input, false, map);
    }
}

// 처음엔 0이었다가 서서히 빨라져서 최고 속도에 닿아요. 넘지는 않아요.
TEST(Mover, AcceleratesToMaxSpeed) {
    const auto map = open_map();
    Mover mover({320.0f, 320.0f}, Direction::Down, MovementTuning{});
    mover.step(kDt, {1.0f, 0.0f}, false, map);
    EXPECT_GT(mover.speed(), 0.0f);
    EXPECT_LT(mover.speed(), mover.tuning().max_speed);  // 첫 박자부터 최고 속도는 아니에요
    run(mover, 30, {1.0f, 0.0f}, map);
    EXPECT_NEAR(mover.speed(), mover.tuning().max_speed, 1e-3f);
    EXPECT_EQ(mover.state(), Mover::State::Walking);
}

// 대각선 최고 속도 = 똑바로 갈 때 최고 속도 (더 빠르면 안 돼요)
TEST(Mover, DiagonalIsNotFaster) {
    const auto map = open_map();
    Mover straight({320.0f, 320.0f}, Direction::Down, MovementTuning{});
    Mover diagonal({320.0f, 320.0f}, Direction::Down, MovementTuning{});
    run(straight, 60, {1.0f, 0.0f}, map);
    run(diagonal, 60, {1.0f, 1.0f}, map);  // 일부러 길이 1.41짜리 입력을 줘도
    EXPECT_NEAR(diagonal.speed(), straight.speed(), 1e-3f);
}

// 키를 떼면 서서히 멈춰서 Idle이 돼요.
TEST(Mover, DeceleratesToIdle) {
    const auto map = open_map();
    Mover mover({320.0f, 320.0f}, Direction::Down, MovementTuning{});
    run(mover, 30, {0.0f, 1.0f}, map);
    run(mover, 30, {0.0f, 0.0f}, map);
    EXPECT_FLOAT_EQ(mover.speed(), 0.0f);
    EXPECT_EQ(mover.state(), Mover::State::Idle);
}

// 구르기: 시작하면 구르기 속도, 끝나면 걷기로 돌아오고, 쉬는 시간 동안은 못 굴러요.
TEST(Mover, DodgeIsFastShortAndHasCooldown) {
    const auto map = open_map();
    const MovementTuning tuning;
    Mover mover({320.0f, 320.0f}, Direction::Right, tuning);
    EXPECT_TRUE(mover.can_dodge());
    EXPECT_TRUE(mover.step(kDt, {0.0f, 0.0f}, true, map));  // 구르기 시작
    EXPECT_EQ(mover.state(), Mover::State::Dodging);
    EXPECT_NEAR(mover.speed(), tuning.dodge_speed, 1e-3f);
    EXPECT_GT(mover.velocity().x, 0.0f);  // 방향키가 없으면 바라보는 쪽(오른쪽)으로
    EXPECT_TRUE(mover.invulnerable());

    // 구르는 동안 또 눌러도 새로 시작하지 않아요.
    EXPECT_FALSE(mover.step(kDt, {0.0f, 0.0f}, true, map));

    const int dodge_steps = static_cast<int>(tuning.dodge_seconds / kDt) + 1;
    run(mover, dodge_steps, {0.0f, 0.0f}, map);
    EXPECT_NE(mover.state(), Mover::State::Dodging);
    EXPECT_FALSE(mover.can_dodge());  // 쉬는 시간

    const int cooldown_steps = static_cast<int>(tuning.dodge_cooldown_seconds / kDt) + 1;
    run(mover, cooldown_steps, {0.0f, 0.0f}, map);
    EXPECT_TRUE(mover.can_dodge());
}

// 구르기 거리 = 약 속도 × 시간 (280 × 0.2 = 56픽셀)
TEST(Mover, DodgeCoversExpectedDistance) {
    const auto map = open_map();
    const MovementTuning tuning;
    Mover mover({200.0f, 320.0f}, Direction::Down, tuning);
    mover.step(kDt, {1.0f, 0.0f}, true, map);
    while (mover.state() == Mover::State::Dodging) {
        mover.step(kDt, {0.0f, 0.0f}, false, map);
    }
    const float travelled = mover.feet().x - 200.0f;
    EXPECT_NEAR(travelled, tuning.dodge_speed * tuning.dodge_seconds, 8.0f);
}

// 벽에 막히면 그 방향 속도가 0이 돼요 (벽에 계속 밀어붙이지 않아요).
TEST(Mover, WallStopsVelocity) {
    auto map = open_map();
    map.walkable[static_cast<std::size_t>(10 * 20 + 12)] = 0;  // (12, 10)에 벽
    Mover mover({370.0f, 340.0f}, Direction::Right, MovementTuning{});
    run(mover, 60, {1.0f, 0.0f}, map);
    EXPECT_FLOAT_EQ(mover.velocity().x, 0.0f);
    EXPECT_LE(mover.feet().x + mover.tuning().body.width * 0.5f, 384.0f);
    EXPECT_EQ(mover.facing(), Direction::Right);  // 막혀도 누르는 쪽을 봐요
}

// 보간: alpha 0이면 전 박자 위치, 1이면 지금 위치, 0.5면 가운데
TEST(Mover, InterpolatesBetweenSteps) {
    const auto map = open_map();
    Mover mover({320.0f, 320.0f}, Direction::Down, MovementTuning{});
    run(mover, 30, {1.0f, 0.0f}, map);
    const Vec2 before = mover.feet();
    mover.step(kDt, {1.0f, 0.0f}, false, map);
    const Vec2 after = mover.feet();
    EXPECT_EQ(mover.interpolated_feet(0.0f), before);
    EXPECT_EQ(mover.interpolated_feet(1.0f), after);
    EXPECT_NEAR(mover.interpolated_feet(0.5f).x, (before.x + after.x) * 0.5f, 1e-4f);
}

// 순간이동하면 중간 모습 없이 바로 그 자리, 멈춘 상태예요.
TEST(Mover, TeleportResetsMotion) {
    const auto map = open_map();
    Mover mover({320.0f, 320.0f}, Direction::Down, MovementTuning{});
    run(mover, 30, {1.0f, 0.0f}, map);
    mover.teleport({100.0f, 100.0f});
    EXPECT_EQ(mover.interpolated_feet(0.0f), (Vec2{100.0f, 100.0f}));
    EXPECT_FLOAT_EQ(mover.speed(), 0.0f);
    EXPECT_EQ(mover.state(), Mover::State::Idle);
}

}  // namespace
