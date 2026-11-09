// 이 파일은 무엇을 하나요?
// 먼지 만들기(DustEmitter)가 걸을 때 조금씩, 구를 때 한꺼번에 먼지를 만드는지 숙제 검사해요.
// 먼지 모양은 효과 파일이 정하니까, 여기서는 "언제, 몇 번" 만드는지만 봐요.
#include "game/presentation/dust_emitter.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::ParticleEffect;
using wind::engine::ParticlePool;
using wind::engine::ParticleRandom;
using wind::game::DustEmitter;

ParticleEffect make_effect(std::uint32_t burst) {
    ParticleEffect e;
    e.lifetime = {5.0f, 5.0f};
    e.burst = burst;
    return e;
}

// 천천히(살금살금) 걸으면 먼지가 안 나고, 빨리 걸으면 거리마다 나요.
TEST(DustEmitter, WalkPuffsByDistance) {
    const auto walk = make_effect(1);
    DustEmitter emitter(&walk, nullptr);
    ParticleRandom random(1);
    ParticlePool pool(64);
    for (int i = 0; i < 60; ++i) {
        emitter.walk({0.0f, 0.0f}, {20.0f, 0.0f}, 1.0f / 60.0f, random, pool);
    }
    EXPECT_EQ(pool.size(), 0u);
    for (int i = 0; i < 60; ++i) {
        emitter.walk({0.0f, 0.0f}, {110.0f, 0.0f}, 1.0f / 60.0f, random, pool);
    }
    // 1초에 110픽셀 ÷ 14픽셀마다 = 7번 정도
    EXPECT_GE(pool.size(), 6u);
    EXPECT_LE(pool.size(), 8u);
}

// 구르기를 시작하면 효과의 burst 개수만큼 한꺼번에 나와요.
TEST(DustEmitter, DodgeBurstUsesEffectCount) {
    const auto dodge = make_effect(10);
    DustEmitter emitter(nullptr, &dodge);
    ParticleRandom random(1);
    ParticlePool pool(64);
    emitter.dodge_burst({0.0f, 0.0f}, {1.0f, 0.0f}, random, pool);
    EXPECT_EQ(pool.size(), 10u);
}

// 효과 파일을 못 읽었으면(nullptr) 먼지를 안 만들 뿐, 멈추지 않아요.
TEST(DustEmitter, MissingEffectsAreSkipped) {
    DustEmitter emitter(nullptr, nullptr);
    ParticleRandom random(1);
    ParticlePool pool(8);
    for (int i = 0; i < 60; ++i) {
        emitter.walk({0.0f, 0.0f}, {110.0f, 0.0f}, 1.0f / 60.0f, random, pool);
    }
    emitter.dodge_burst({0.0f, 0.0f}, {1.0f, 0.0f}, random, pool);
    EXPECT_EQ(pool.size(), 0u);
}

}  // namespace
