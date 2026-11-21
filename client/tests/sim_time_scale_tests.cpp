// 이 파일은 무엇을 하나요?
// 시뮬레이션 시간 배수(SimTimeScale)와, 슬로모션이 시뮬레이션 결과를 깨뜨리지 않는지(12b) 숙제 검사해요.
// - 히트스톱(0)과 슬로모션(0.3~1)을 곱해요. 쓰러진 박자에 둘 다 걸리면 먼저 멈췄다가 느리게 이어져요
// - 느린 동안 주인공(넉백)도 빛 탄도 순간이동 없이 "같은 길"을 천천히 가요
#include "game/combat/sim_time_scale.hpp"

#include "game/combat/projectile.hpp"
#include "game/entities/player_motion.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Vec2;
using wind::game::SimTimeScale;

constexpr float kDt = 1.0f / 60.0f;

TEST(SimTimeScale, NormalIsOne) {
    SimTimeScale time;
    EXPECT_FLOAT_EQ(time.scale(), 1.0f);
    time.update(kDt, 0.0f, false);  // 아무 일도 없으면 그대로
    EXPECT_FLOAT_EQ(time.scale(), 1.0f);
}

TEST(SimTimeScale, HitstopThenSlowMotionOnFatalHit) {
    SimTimeScale time;
    time.update(kDt, 0.05f, true);    // 마지막 한 방: 멈칫 0.05초(3박자) + 슬로모션
    EXPECT_FLOAT_EQ(time.scale(), 0.0f);  // 먼저 멈칫
    for (int i = 0; i < 3; ++i) time.update(kDt, 0.0f, false);
    EXPECT_FALSE(time.hit_stop().active());
    EXPECT_TRUE(time.slow_motion().active());
    EXPECT_GT(time.scale(), 0.29f);   // 멈칫이 풀리면 느린 시간 (0.3배 근처)
    EXPECT_LT(time.scale(), 0.35f);
    for (int i = 0; i < 120; ++i) time.update(kDt, 0.0f, false);  // 2초 뒤
    EXPECT_FLOAT_EQ(time.scale(), 1.0f);
}

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 40;
    map.height = 20;
    map.walkable.assign(static_cast<std::size_t>(40 * 20), 1);
    return map;
}

// 12b 완료 조건: 슬로모션이 위치를 깨뜨리지 않아요.
// 빛 탄은 일정한 빠르기라, 느린 시간으로 간 거리 합 = 보통 시간으로 같은 "시뮬레이션 시간"만큼 간 거리예요.
TEST(SimTimeScale, SlowMotionKeepsProjectilePathExact) {
    const auto map = open_map();
    wind::game::ProjectileShot shot;
    shot.ground = {100.0f, 300.0f};
    shot.data.speed = 360.0f;
    shot.data.range = 1000.0f;
    shot.data.width = 6.0f;
    shot.data.height = 6.0f;
    wind::game::Projectile slowed(shot);
    SimTimeScale time;
    time.update(kDt, 0.0f, true);  // 쓰러짐: 다음 박자부터 느려요
    const auto none = [](const wind::engine::RectF&) { return false; };
    double sim_seconds = 0.0;      // 느린 쪽이 실제로 흘려보낸 시뮬레이션 시간
    Vec2 previous = slowed.ground();
    for (int i = 0; i < 30; ++i) {
        const float sim_dt = kDt * time.scale();
        slowed.step(sim_dt, map, none);
        sim_seconds += sim_dt;
        EXPECT_GE(slowed.ground().x, previous.x);                         // 뒤로 가지 않아요
        EXPECT_LE(slowed.ground().x - previous.x, 360.0f * kDt + 1e-3f);  // 한 박자에 보통보다 멀리 안 가요
        previous = slowed.ground();
        time.update(kDt, 0.0f, false);
    }
    EXPECT_LT(sim_seconds, 30.0 * kDt);  // 정말 느렸어요
    EXPECT_NEAR(slowed.ground().x, 100.0f + 360.0f * static_cast<float>(sim_seconds), 1e-2f);
    EXPECT_FLOAT_EQ(slowed.ground().y, 300.0f);  // 옆으로 새지 않아요
}

// 쓰러진 주인공의 넉백도 느린 시간에서 같은 거리만큼 밀려요 (빠르기만 느려져요).
TEST(SimTimeScale, SlowMotionKeepsKnockbackDistance) {
    const auto map = open_map();
    const auto make = [] {
        return wind::game::PlayerMotion({300.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                                        wind::game::StaminaTuning{}, 0.15);
    };
    auto slowed = make();
    auto plain = make();
    (void)slowed.take_hit(1000, {-1.0f, 0.0f}, 160.0f);  // 마지막 한 방: 왼쪽으로 밀려요
    (void)plain.take_hit(1000, {-1.0f, 0.0f}, 160.0f);
    ASSERT_TRUE(slowed.dead());
    SimTimeScale time;
    time.update(kDt, 0.0f, true);
    for (int i = 0; i < 200; ++i) {  // 넉백이 다 끝날 만큼
        slowed.step(kDt * time.scale(), {}, false, map);
        time.update(kDt, 0.0f, false);
        plain.step(kDt, {}, false, map);
    }
    EXPECT_LT(slowed.mover().feet().x, 300.0f);  // 실제로 밀렸고
    EXPECT_NEAR(slowed.mover().feet().x, plain.mover().feet().x, 0.5f);  // 같은 자리에서 멈춰요
}

}  // namespace
