// 이 파일은 무엇을 하나요?
// 파티클 통(ParticlePool)이 점을 움직이고, 치우고, 넘치지 않게 막는지 숙제 검사(테스트)를 해요.
#include "engine/fx/particle_pool.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Particle;
using wind::engine::ParticlePool;

Particle make(float lifetime) {
    Particle p;
    p.velocity = {10.0f, 0.0f};
    p.lifetime = lifetime;
    return p;
}

// 점은 속도만큼 움직이고, 수명이 다하면 사라져요.
TEST(ParticlePool, MovesAndExpires) {
    ParticlePool pool(8);
    pool.spawn(make(0.5f));
    pool.update(0.25f);
    ASSERT_EQ(pool.size(), 1u);
    EXPECT_NEAR(pool.particles()[0].position.x, 2.5f, 1e-4f);
    pool.update(0.3f);
    EXPECT_EQ(pool.size(), 0u);
}

// 통이 꽉 차면 새 점은 버리고 몇 개 버렸는지 세요.
TEST(ParticlePool, DropsWhenFull) {
    ParticlePool pool(2);
    EXPECT_TRUE(pool.spawn(make(1.0f)));
    EXPECT_TRUE(pool.spawn(make(1.0f)));
    EXPECT_FALSE(pool.spawn(make(1.0f)));
    EXPECT_EQ(pool.size(), 2u);
    EXPECT_EQ(pool.dropped(), 1u);
}

// 공기 저항이 있으면 점점 느려지고, 중력은 아래로 당겨요.
TEST(ParticlePool, DragAndGravity) {
    ParticlePool pool(1);
    Particle p = make(10.0f);
    p.drag = 2.0f;
    p.gravity = 100.0f;
    pool.spawn(p);
    pool.update(0.5f);
    const auto& q = pool.particles()[0];
    EXPECT_LT(q.velocity.x, 10.0f);
    EXPECT_GT(q.velocity.y, 0.0f);
}

}  // namespace
