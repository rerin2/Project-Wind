// 이 파일은 무엇을 하나요?
// 먼지 만들기(DustEmitter)가 걸을 때 조금씩, 구를 때 한꺼번에 먼지를 만드는지 숙제 검사해요.
#include "game/presentation/dust_emitter.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::ParticlePool;
using wind::game::DustEmitter;

// 천천히(살금살금) 걸으면 먼지가 안 나고, 빨리 걸으면 거리마다 나요.
TEST(DustEmitter, WalkPuffsByDistance) {
    DustEmitter emitter(1);
    ParticlePool pool(64);
    for (int i = 0; i < 60; ++i) {
        emitter.walk({0.0f, 0.0f}, {20.0f, 0.0f}, 1.0f / 60.0f, pool);
    }
    EXPECT_EQ(pool.size(), 0u);
    for (int i = 0; i < 60; ++i) {
        emitter.walk({0.0f, 0.0f}, {110.0f, 0.0f}, 1.0f / 60.0f, pool);
    }
    // 1초에 110픽셀 ÷ 14픽셀마다 = 7번 정도
    EXPECT_GE(pool.size(), 6u);
    EXPECT_LE(pool.size(), 8u);
}

// 구르기를 시작하면 먼지가 한꺼번에 여러 개 나와요.
TEST(DustEmitter, DodgeBurstSpawnsMany) {
    DustEmitter emitter(1);
    ParticlePool pool(64);
    emitter.dodge_burst({0.0f, 0.0f}, {1.0f, 0.0f}, pool);
    EXPECT_EQ(pool.size(), 10u);
}

}  // namespace
