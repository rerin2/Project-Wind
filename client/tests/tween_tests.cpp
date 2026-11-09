// 이 파일은 무엇을 하나요?
// 트윈(숫자를 부드럽게 바꾸는 도구)이 시간에 맞춰 값을 바꾸는지 숙제 검사(테스트)를 해요.
#include "engine/tween/tween.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Ease;
using wind::engine::Tween;

// 255 -> 0을 1초 동안 일정하게: 0.25초 뒤엔 191.25, 끝나면 0
TEST(Tween, LinearProgress) {
    Tween t(255.0f, 0.0f, 1.0f, Ease::Linear);
    EXPECT_FLOAT_EQ(t.value(), 255.0f);
    t.update(0.25f);
    EXPECT_FLOAT_EQ(t.value(), 191.25f);
    EXPECT_FALSE(t.finished());
    t.update(5.0f);  // 시간이 넘쳐도 끝 값에서 멈춰요
    EXPECT_FLOAT_EQ(t.value(), 0.0f);
    EXPECT_TRUE(t.finished());
}

// restart하면 처음 값으로 돌아가요.
TEST(Tween, RestartGoesBackToStart) {
    Tween t(0.0f, 10.0f, 0.5f, Ease::OutQuad);
    t.update(0.5f);
    ASSERT_TRUE(t.finished());
    t.restart();
    EXPECT_FLOAT_EQ(t.value(), 0.0f);
    EXPECT_FALSE(t.finished());
}

// 길이 0인 트윈은 처음부터 끝 값이에요. (0으로 나누기 같은 사고가 없어야 해요)
TEST(Tween, ZeroDurationIsFinished) {
    Tween t(3.0f, 9.0f, 0.0f, Ease::InCubic);
    EXPECT_TRUE(t.finished());
    EXPECT_FLOAT_EQ(t.value(), 9.0f);
}

// 시간을 거꾸로(음수) 줘도 뒤로 가지 않아요.
TEST(Tween, NegativeTimeIsIgnored) {
    Tween t(0.0f, 1.0f, 1.0f, Ease::Linear);
    t.update(0.5f);
    t.update(-1.0f);
    EXPECT_FLOAT_EQ(t.value(), 0.5f);
}

}  // namespace
