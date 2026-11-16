// 이 파일은 무엇을 하나요?
// 히트스톱(HitStop)과, 히트스톱이 시뮬레이션 결과를 깨뜨리지 않는지(10단계 완료 조건)를 숙제 검사해요.
// - 0.05초 멈춤 = 3박자 동안 time_scale 0, 그다음 1
// - 겹치면 더 긴 쪽만 (더하지 않아요), 이상한 값은 무시
// - 멈춘 동안 주인공은 한 픽셀도 안 움직이고, 멈춘 박자 수만큼 늦어질 뿐 같은 길을 같은 위치로 가요
// - 멈춘 동안 빛 탄도 그 자리에 있고, 풀리면 같은 거리를 날아요
#include "game/combat/hit_stop.hpp"

#include "game/combat/projectile.hpp"
#include "game/entities/player_motion.hpp"

#include <gtest/gtest.h>

#include <limits>

namespace {

using wind::engine::Vec2;
using wind::game::HitStop;
using wind::game::PlayerMotion;

constexpr float kDt = 1.0f / 60.0f;

TEST(HitStop, FreezesForWholeSteps) {
    HitStop stop;
    EXPECT_FLOAT_EQ(stop.time_scale(), 1.0f);
    stop.trigger(0.05f);  // 3박자 (0.05 / 0.0167)
    int frozen = 0;
    for (int i = 0; i < 10; ++i) {
        if (stop.time_scale() == 0.0f) ++frozen;
        stop.update(kDt);
    }
    EXPECT_EQ(frozen, 3);
    EXPECT_FALSE(stop.active());
}

TEST(HitStop, OverlapKeepsTheLongerNotTheSum) {
    HitStop stop;
    stop.trigger(0.09f);
    stop.update(kDt);
    stop.trigger(0.05f);  // 남은 0.073보다 짧아서 무시
    EXPECT_NEAR(stop.remaining(), 0.09f - kDt, 1e-5f);
    stop.trigger(0.2f);   // 더 길면 늘어나요
    EXPECT_FLOAT_EQ(stop.remaining(), 0.2f);
}

TEST(HitStop, IgnoresBadValuesAndCaps) {
    HitStop stop;
    stop.trigger(0.0f);
    stop.trigger(-1.0f);
    stop.trigger(std::numeric_limits<float>::quiet_NaN());
    EXPECT_FALSE(stop.active());
    stop.trigger(100.0f);  // 아무리 길어도 0.5초까지
    EXPECT_FLOAT_EQ(stop.remaining(), 0.5f);
}

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 40;
    map.height = 20;
    map.walkable.assign(static_cast<std::size_t>(40 * 20), 1);
    return map;
}

PlayerMotion make_player() {
    return PlayerMotion({100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15);
}

// 필드가 하는 것과 같은 순서: 배수를 읽고 -> 시뮬레이션 시간 = 박자 × 배수 -> 진짜 시간으로 멈춤 줄이기
// 10단계 완료 조건: 히트스톱이 위치를 깨뜨리지 않아요.
TEST(HitStop, DoesNotBreakMovement) {
    const auto map = open_map();
    auto stopped = make_player();
    auto plain = make_player();
    HitStop stop;
    const Vec2 right{1.0f, 0.0f};
    for (int i = 0; i < 20; ++i) {   // 둘 다 20박자 걷기
        stopped.step(kDt * stop.time_scale(), right, false, map);
        stop.update(kDt);
        plain.step(kDt, right, false, map);
    }
    ASSERT_EQ(stopped.mover().feet().x, plain.mover().feet().x);

    stop.trigger(0.05f);  // 3박자 멈춤
    const Vec2 before = stopped.mover().feet();
    for (int i = 0; i < 3; ++i) {
        stopped.step(kDt * stop.time_scale(), right, false, map);
        stop.update(kDt);
        EXPECT_EQ(stopped.mover().feet().x, before.x) << "moved during hitstop at " << i;
        EXPECT_EQ(stopped.mover().interpolated_feet(0.0f).x, before.x);  // 보간도 제자리 (덜덜 떨지 않게)
    }
    for (int i = 0; i < 30; ++i) {   // 멈춘 쪽은 30박자, 안 멈춘 쪽은 30박자 더
        stopped.step(kDt * stop.time_scale(), right, false, map);
        stop.update(kDt);
        plain.step(kDt, right, false, map);
    }
    // 멈춘 3박자만큼 늦을 뿐, 같은 박자 수를 걸었으면 같은 위치예요 (순간이동·건너뛰기 없음)
    EXPECT_EQ(stopped.mover().feet().x, plain.mover().feet().x);
    EXPECT_EQ(stopped.mover().velocity().x, plain.mover().velocity().x);
}

// 빛 탄도 멈춘 동안 그 자리에 있고, 풀린 뒤에는 똑같이 날아요.
TEST(HitStop, FreezesProjectiles) {
    const auto map = open_map();
    wind::game::ProjectileShot shot;
    shot.ground = {100.0f, 300.0f};
    shot.data.speed = 360.0f;
    shot.data.range = 240.0f;
    shot.data.width = 6.0f;
    shot.data.height = 6.0f;
    wind::game::Projectile shard(shot);
    HitStop stop;
    stop.trigger(0.05f);
    const auto none = [](const wind::engine::RectF&) { return false; };
    for (int i = 0; i < 3; ++i) {
        shard.step(kDt * stop.time_scale(), map, none);
        stop.update(kDt);
    }
    EXPECT_FLOAT_EQ(shard.ground().x, 100.0f);
    EXPECT_FLOAT_EQ(shard.previous_ground().x, 100.0f);
    shard.step(kDt * stop.time_scale(), map, none);
    EXPECT_NEAR(shard.ground().x, 106.0f, 1e-3f);
}

}  // namespace
