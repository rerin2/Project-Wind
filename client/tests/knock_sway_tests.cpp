// 이 파일은 무엇을 하나요?
// 용수철 흔들림 넉백(KnockSway)과 허수아비 넉백을 숙제 검사(테스트)해요.
// - 맞은 쪽으로 밀렸다가 제자리로 돌아와요
// - 아무리 세게 맞아도 8픽셀보다 멀리 안 가요
// - 시간이 0이면(히트스톱) 그대로 멈춰 있어요
// - 허수아비는 밀린 만큼 맞는 상자도 같이 움직이고, 발 위치(칸)는 그대로예요
#include "game/combat/knock_sway.hpp"

#include "game/combat/training_dummy.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

using wind::engine::Vec2;
using wind::game::KnockSway;

constexpr float kDt = 1.0f / 60.0f;

// 롱소드 1타(넉백 90) 오른쪽: 몇 픽셀 밀렸다가 1초 안에 제자리
TEST(KnockSway, PushesThenSettles) {
    KnockSway sway;
    sway.push({1.0f, 0.0f}, 90.0f);
    float peak = 0.0f;
    for (int i = 0; i < 60; ++i) {
        sway.step(kDt);
        peak = std::max(peak, sway.offset().x);
    }
    EXPECT_GT(peak, 2.0f);   // 눈에 보일 만큼
    EXPECT_LE(peak, 8.0f);
    EXPECT_TRUE(sway.resting());
    EXPECT_FLOAT_EQ(sway.offset().x, 0.0f);
}

TEST(KnockSway, NeverPastMaxOffset) {
    KnockSway sway;
    sway.push({0.0f, 1.0f}, 1000.0f);
    for (int i = 0; i < 30; ++i) {
        sway.step(kDt);
        ASSERT_LE(wind::engine::length(sway.offset()), 8.0f + 1e-4f);
    }
}

TEST(KnockSway, ZeroTimeFreezes) {
    KnockSway sway;
    sway.push({1.0f, 0.0f}, 160.0f);
    sway.step(kDt);
    const Vec2 at = sway.offset();
    for (int i = 0; i < 5; ++i) sway.step(0.0f);
    EXPECT_EQ(sway.offset().x, at.x);
}

TEST(KnockSway, IgnoresNoDirectionOrNoForce) {
    KnockSway sway;
    sway.push({0.0f, 0.0f}, 90.0f);
    sway.push({1.0f, 0.0f}, 0.0f);
    sway.step(kDt);
    EXPECT_TRUE(sway.resting());
}

// 허수아비: 맞는 상자는 밀린 만큼 옮겨지고, 발(칸)은 그대로
TEST(KnockSway, DummyHurtboxFollowsBody) {
    wind::combat::TrainingDummyData data;
    data.max_hp = 100;
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    wind::game::TrainingDummy dummy(1, "d", data, {100.0f, 200.0f});
    dummy.push({1.0f, 0.0f}, 160.0f);
    dummy.step(kDt);
    dummy.step(kDt);
    EXPECT_GT(dummy.body_offset().x, 0.5f);
    EXPECT_FLOAT_EQ(dummy.hurtbox().x, 90.0f + dummy.body_offset().x);
    EXPECT_FLOAT_EQ(dummy.feet().x, 100.0f);
}

}  // namespace
