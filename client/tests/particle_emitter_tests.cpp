// 이 파일은 무엇을 하나요?
// 계속 뿜는 방출기가 1초에 rate개씩, 정해진 영역 안에서 점을 만드는지 숙제 검사(테스트)를 해요.
#include "engine/fx/particle_emitter.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::ParticleEffect;
using wind::engine::ParticleEmitter;
using wind::engine::ParticlePool;
using wind::engine::ParticleRandom;

ParticleEffect make_effect(float rate) {
    ParticleEffect e;
    e.lifetime = {10.0f, 10.0f};
    e.rate = rate;
    return e;
}

// 1초에 10개씩: 1초 동안(1/60초 × 60번) 10개가 생겨야 해요.
TEST(ParticleEmitter, EmitsRatePerSecond) {
    const auto effect = make_effect(10.0f);
    ParticleEmitter emitter(effect);
    ParticleRandom random(3);
    ParticlePool pool(100);
    std::uint32_t total = 0;
    for (int i = 0; i < 60; ++i) {
        total += emitter.update(1.0f / 60.0f, {0.0f, 0.0f, 100.0f, 50.0f}, random, pool);
    }
    EXPECT_GE(total, 9u);
    EXPECT_LE(total, 10u);
}

// 1초에 0.5개: 1초로는 부족하고, 2초가 지나야 1개가 생겨요 (모아 뒀다가 만들어요).
TEST(ParticleEmitter, AccumulatesFractions) {
    const auto effect = make_effect(0.5f);
    ParticleEmitter emitter(effect);
    ParticleRandom random(3);
    ParticlePool pool(10);
    // 0.25초마다 0.125개씩 모여요: 7번(1.75초)까지는 0개, 8번째(2초)에 1개
    for (int i = 0; i < 7; ++i) {
        EXPECT_EQ(emitter.update(0.25f, {0.0f, 0.0f, 10.0f, 10.0f}, random, pool), 0u) << i;
    }
    EXPECT_EQ(emitter.update(0.25f, {0.0f, 0.0f, 10.0f, 10.0f}, random, pool), 1u);
}

// 점은 정해 준 네모 안에서만 생겨요.
TEST(ParticleEmitter, SpawnsInsideArea) {
    const auto effect = make_effect(1000.0f);
    ParticleEmitter emitter(effect);
    ParticleRandom random(3);
    ParticlePool pool(500);
    emitter.update(0.2f, {100.0f, 200.0f, 50.0f, 20.0f}, random, pool);
    ASSERT_GT(pool.size(), 0u);
    for (const auto& p : pool.particles()) {
        EXPECT_GE(p.position.x, 100.0f);
        EXPECT_LE(p.position.x, 150.0f);
        EXPECT_GE(p.position.y, 200.0f);
        EXPECT_LE(p.position.y, 220.0f);
    }
}

// 게임이 오래 멈췄다 돌아와도(5초) 한꺼번에 너무 많이 뿜지 않아요 (0.25초 몫만).
TEST(ParticleEmitter, LimitsCatchUp) {
    const auto effect = make_effect(100.0f);
    ParticleEmitter emitter(effect);
    ParticleRandom random(3);
    ParticlePool pool(1000);
    EXPECT_LE(emitter.update(5.0f, {0.0f, 0.0f, 10.0f, 10.0f}, random, pool), 25u);
}

// 뿜는 양 배율: 0.5배면 반, 0배면 하나도 안 나와요 (날씨가 서서히 그칠 때).
TEST(ParticleEmitter, RateScaleMultipliesOutput) {
    const auto effect = make_effect(100.0f);
    ParticleEmitter emitter(effect);
    ParticleRandom random(3);
    ParticlePool pool(1000);
    EXPECT_EQ(emitter.update(0.2f, {0.0f, 0.0f, 10.0f, 10.0f}, random, pool, 0.5f), 10u);
    EXPECT_EQ(emitter.update(0.2f, {0.0f, 0.0f, 10.0f, 10.0f}, random, pool, 0.0f), 0u);
}

}  // namespace
