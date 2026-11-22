// 이 파일은 무엇을 하나요?
// 룬 알갱이 연출 검사 (13c): 룬 양에 맞는 개수, 주인공에게 빨려 들어 사라지기, 멀면 2초 뒤 사라지기.
#include "game/presentation/rune_orbs.hpp"

#include <gtest/gtest.h>

using wind::game::RuneOrbs;

namespace {
constexpr float kDt = 1.0f / 60.0f;
}

TEST(RuneOrbs, CountFollowsAmount) {
    wind::engine::ParticleRandom random(3);
    RuneOrbs orbs;
    orbs.spawn({0.0f, 0.0f}, 12, random);   // 슬라임 12룬 -> 3개
    EXPECT_EQ(orbs.count(), 3u);
    orbs.spawn({0.0f, 0.0f}, 400, random);  // 많아도 한 번에 10개까지
    EXPECT_EQ(orbs.count(), 13u);
    orbs.spawn({0.0f, 0.0f}, 0, random);    // 0룬은 없음
    EXPECT_EQ(orbs.count(), 13u);
}

TEST(RuneOrbs, FlyToHeroAndVanish) {
    wind::engine::ParticleRandom random(3);
    RuneOrbs orbs;
    orbs.spawn({100.0f, 100.0f}, 12, random);
    for (int i = 0; i < 90 && orbs.count() > 0; ++i) orbs.update(kDt, {140.0f, 90.0f});  // 40픽셀 옆의 주인공
    EXPECT_EQ(orbs.count(), 0u);  // 1.5초 안에 다 빨려 들어가요
}

TEST(RuneOrbs, GiveUpAfterTwoSeconds) {
    wind::engine::ParticleRandom random(3);
    RuneOrbs orbs;
    orbs.spawn({0.0f, 0.0f}, 12, random);
    for (int i = 0; i < 125; ++i) orbs.update(kDt, {100000.0f, 0.0f});  // 아주 먼 곳
    EXPECT_EQ(orbs.count(), 0u);
}
