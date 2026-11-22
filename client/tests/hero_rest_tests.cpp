// 이 파일은 무엇을 하나요?
// 쉬기 시간표 검사 (13b): 쿠션 가운데로 옮겨 가기 -> 앉기 -> 일어나기 요청 -> 기지개 -> 끝.
#include "game/entities/hero_rest.hpp"

#include <gtest/gtest.h>

using wind::engine::Vec2;
using wind::game::HeroRest;

namespace {
constexpr float kDt = 1.0f / 60.0f;
}

TEST(HeroRest, SettlesOntoSeatThenSits) {
    HeroRest rest;
    Vec2 feet{100.0f, 100.0f};
    const Vec2 seat{112.0f, 104.0f};
    rest.begin(seat);
    EXPECT_EQ(rest.phase(), HeroRest::Phase::Settling);
    for (int i = 0; i < 20 && rest.phase() == HeroRest::Phase::Settling; ++i) feet = feet + rest.step(kDt, feet);
    EXPECT_EQ(rest.phase(), HeroRest::Phase::Sitting);  // 0.25초 안에 앉아요
    EXPECT_NEAR(feet.x, seat.x, 1e-3f);                 // 딱 쿠션 자리
    EXPECT_NEAR(feet.y, seat.y, 1e-3f);
}

TEST(HeroRest, StandUpOnlyWhileSitting) {
    HeroRest rest;
    rest.begin({0.0f, 0.0f});
    EXPECT_FALSE(rest.stand_up());  // 옮겨 가는 중에는 못 일어나요
    rest.sit_now({0.0f, 0.0f});
    EXPECT_TRUE(rest.stand_up());
    EXPECT_EQ(rest.phase(), HeroRest::Phase::Standing);
    EXPECT_FALSE(rest.stand_up());  // 이미 일어나는 중
}

TEST(HeroRest, StandingEndsAfterStretch) {
    HeroRest rest;
    rest.sit_now({0.0f, 0.0f});
    ASSERT_TRUE(rest.stand_up());
    for (int i = 0; i < 35; ++i) (void)rest.step(kDt, {});  // 0.58초: 아직 기지개
    EXPECT_TRUE(rest.active());
    for (int i = 0; i < 3; ++i) (void)rest.step(kDt, {});   // 0.63초: 끝
    EXPECT_FALSE(rest.active());
}

TEST(HeroRest, HitstopFreezesRest) {
    HeroRest rest;
    rest.begin({10.0f, 0.0f});
    const Vec2 delta = rest.step(0.0f, {});  // 멈칫(0)이면 아무것도 안 바뀌어요
    EXPECT_FLOAT_EQ(delta.x, 0.0f);
    EXPECT_FLOAT_EQ(rest.phase_seconds(), 0.0f);
}
