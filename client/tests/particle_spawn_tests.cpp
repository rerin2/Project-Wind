// 이 파일은 무엇을 하나요?
// 요리법을 보고 점을 만드는 함수들이 범위를 지키는지 숙제 검사(테스트)를 해요.
#include "engine/fx/particle_spawn.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::ParticleEffect;
using wind::engine::ParticlePool;
using wind::engine::ParticleRandom;

ParticleEffect make_effect() {
    ParticleEffect e;
    e.lifetime = {1.0f, 2.0f};
    e.speed = {10.0f, 20.0f};
    e.start_size = {3.0f, 5.0f};
    e.spread = 0.0f;
    e.burst = 5;
    return e;
}

// 만든 점의 수명·크기·빠르기는 요리법 범위 안이에요. 흩어짐 0이면 방향은 정확히 그쪽.
TEST(ParticleSpawn, RespectsRanges) {
    const auto effect = make_effect();
    ParticleRandom random(1);
    for (int i = 0; i < 100; ++i) {
        const auto p = wind::engine::make_particle(effect, {0.0f, 0.0f}, 90.0f, random);
        EXPECT_GE(p.lifetime, 1.0f);
        EXPECT_LE(p.lifetime, 2.0f);
        EXPECT_GE(p.start_size, 3.0f);
        EXPECT_LE(p.start_size, 5.0f);
        const float speed = wind::engine::length(p.velocity);
        EXPECT_GE(speed, 10.0f - 1e-3f);
        EXPECT_LE(speed, 20.0f + 1e-3f);
        EXPECT_NEAR(p.velocity.x, 0.0f, 1e-3f);  // 90도 = 정확히 아래
        EXPECT_GT(p.velocity.y, 0.0f);
    }
}

// 한꺼번에 터뜨리기: burst개만큼 통에 들어가요. 통이 작으면 들어간 만큼만 세요.
TEST(ParticleSpawn, BurstAddsCount) {
    const auto effect = make_effect();
    ParticleRandom random(1);
    ParticlePool pool(3);
    EXPECT_EQ(wind::engine::emit_burst(effect, {0.0f, 0.0f}, 0.0f, random, pool), 3u);
    EXPECT_EQ(pool.dropped(), 2u);
}

// 화살표 방향 → 각도: 오른쪽 0도, 아래 90도, 위 -90도
TEST(ParticleSpawn, DirectionToDegrees) {
    EXPECT_NEAR(wind::engine::direction_to_degrees({1.0f, 0.0f}), 0.0f, 1e-4f);
    EXPECT_NEAR(wind::engine::direction_to_degrees({0.0f, 1.0f}), 90.0f, 1e-4f);
    EXPECT_NEAR(wind::engine::direction_to_degrees({0.0f, -1.0f}), -90.0f, 1e-4f);
}

// 같은 씨앗이면 같은 점이 나와요 (테스트에서 결과를 맞춰 볼 수 있어요).
TEST(ParticleSpawn, SameSeedSameParticle) {
    const auto effect = make_effect();
    ParticleRandom a(42);
    ParticleRandom b(42);
    const auto pa = wind::engine::make_particle(effect, {0.0f, 0.0f}, 0.0f, a);
    const auto pb = wind::engine::make_particle(effect, {0.0f, 0.0f}, 0.0f, b);
    EXPECT_EQ(pa.velocity, pb.velocity);
    EXPECT_FLOAT_EQ(pa.lifetime, pb.lifetime);
}

}  // namespace
