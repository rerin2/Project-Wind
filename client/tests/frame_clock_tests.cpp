// 이 파일은 무엇을 하나요?
// 박자 시계(FixedStepClock)가 맞게 박자를 세는지 숙제 검사(테스트)를 해요.
// - 시간이 조금씩 흐르면 모아 두었다가 한 박자가 차면 계산 1번
// - 컴퓨터가 오래 멈췄다 돌아와도 계산을 너무 많이 몰아서 하지 않기
#include "engine/core/frame_clock.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::FixedStepClock;

constexpr double kStep = 1.0 / 60.0;  // 한 박자 = 1/60초

// 반 박자보다 조금 더(0.6박자)가 흐르면 아직 계산 0번, 시간만 저금해요.
// 한 번 더 0.6박자가 흐르면 합이 1.2박자 -> 계산 1번, 남은 0.2박자는 저금통에.
TEST(FixedStepClock, AccumulatesPartialFrames) {
    FixedStepClock clock(kStep, 0.25, 5);
    EXPECT_EQ(clock.advance(kStep * 0.6), 0);
    EXPECT_EQ(clock.advance(kStep * 0.6), 1);
    EXPECT_NEAR(clock.alpha(), 0.2, 1e-9);
}

// 한 번에 3.5박자가 흐르면 계산 3번을 해야 해요.
TEST(FixedStepClock, RunsMultipleStepsForLongFrame) {
    FixedStepClock clock(kStep, 0.25, 5);
    EXPECT_EQ(clock.advance(kStep * 3.5), 3);
}

// 2초나 멈췄다 와도(창을 끌고 있었을 때 등) 최대 5번만 계산하고 나머지는 버려야 해요.
// 그다음 박자는 다시 평소처럼 동작해야 해요.
TEST(FixedStepClock, CapsCatchUpAfterLongStall) {
    FixedStepClock clock(kStep, 0.25, 5);
    EXPECT_EQ(clock.advance(2.0), 5);
    EXPECT_LT(clock.alpha(), 1.0);
    EXPECT_GE(clock.dropped_frames(), 1u);
    // 다음 정상 프레임은 다시 정상적으로 동작한다.
    EXPECT_LE(clock.advance(kStep), 2);
}

// 시간이 거꾸로(-1초) 흘렀다고 해도 무시해야 해요.
TEST(FixedStepClock, IgnoresNegativeTime) {
    FixedStepClock clock(kStep, 0.25, 5);
    EXPECT_EQ(clock.advance(-1.0), 0);
    EXPECT_EQ(clock.alpha(), 0.0);
}

}  // namespace
