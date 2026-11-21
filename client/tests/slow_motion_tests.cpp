// 이 파일은 무엇을 하나요?
// 쓰러질 때 슬로모션(SlowMotion)을 숙제 검사해요. (12b)
// - 처음엔 1배, trigger하면 0.3배에서 시작해 1.5초 동안 1배로 돌아와요 (거꾸로 가지 않아요)
// - 진짜 시간으로 흘러서 정해진 박자 수(90박자) 뒤에 끝나요
// - 이상한 시간·이상한 숫자는 무시하거나 안전한 범위로 잘라요
#include "game/combat/slow_motion.hpp"

#include <gtest/gtest.h>

#include <limits>

namespace {

using wind::game::SlowMotion;
using wind::game::SlowMotionTuning;

constexpr float kDt = 1.0f / 60.0f;

TEST(SlowMotion, StartsAtNormalSpeed) {
    const SlowMotion slow;
    EXPECT_FALSE(slow.active());
    EXPECT_FLOAT_EQ(slow.time_scale(), 1.0f);
}

TEST(SlowMotion, RampsFromStartScaleBackToOne) {
    SlowMotion slow;
    slow.trigger();
    EXPECT_TRUE(slow.active());
    EXPECT_FLOAT_EQ(slow.time_scale(), 0.3f);  // 쓰러진 순간 0.3배
    float previous = slow.time_scale();
    int steps = 0;
    while (slow.active() && steps < 1000) {
        slow.update(kDt);
        ++steps;
        EXPECT_GE(slow.time_scale(), previous) << "slowed down again at step " << steps;  // 빨라지기만 해요
        previous = slow.time_scale();
    }
    EXPECT_NEAR(steps, 90, 1);                 // 1.5초 = 90박자 (소수 오차로 한 박자 차이는 괜찮아요)
    EXPECT_FLOAT_EQ(slow.time_scale(), 1.0f);
}

TEST(SlowMotion, StaysSlowLongerThenSpeedsUp) {
    SlowMotion slow;
    slow.trigger();
    slow.update(0.75f);  // 절반 지남
    // InQuad: 절반 지났어도 0.3 + 0.7 × 0.25 = 0.475배 (아직 절반 넘게 느려요)
    EXPECT_NEAR(slow.time_scale(), 0.475f, 1e-4f);
}

TEST(SlowMotion, RetriggerStartsOver) {
    SlowMotion slow;
    slow.trigger();
    slow.update(1.0f);
    slow.trigger();
    EXPECT_FLOAT_EQ(slow.time_scale(), 0.3f);
}

TEST(SlowMotion, IgnoresBadTimeAndClampsTuning) {
    SlowMotion slow;
    slow.trigger();
    slow.update(-1.0f);
    slow.update(std::numeric_limits<float>::quiet_NaN());
    EXPECT_FLOAT_EQ(slow.time_scale(), 0.3f);  // 그대로

    SlowMotion frozen(SlowMotionTuning{0.0f, 1.0f});  // 0배(멈춤)는 안 돼요: 0.05배까지
    frozen.trigger();
    EXPECT_FLOAT_EQ(frozen.time_scale(), 0.05f);

    SlowMotion none(SlowMotionTuning{0.3f, 0.0f});  // 시간 0이면 느려지지 않아요
    none.trigger();
    EXPECT_FALSE(none.active());
    EXPECT_FLOAT_EQ(none.time_scale(), 1.0f);
}

}  // namespace
