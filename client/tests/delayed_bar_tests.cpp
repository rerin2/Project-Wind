// 이 파일은 무엇을 하나요?
// HUD 지연 바를 숙제 검사(테스트)해요.
// - 줄면 바는 바로 줄고, 꼬리는 0.35초 기다렸다가 따라 줄어요
// - 늘면 부드럽게 차올라요
// - 자동 숨김: 가득 찬 채 1.2초 지나면 흐려지고, 쓰면 바로 진해져요
#include "game/ui/delayed_bar.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::DelayedBar;
using wind::game::DelayedBarTuning;

TEST(DelayedBar, DropLeavesTrailThatCatchesUp) {
    DelayedBar bar;
    bar.set(0.6f);
    EXPECT_FLOAT_EQ(bar.value(), 0.6f);
    EXPECT_FLOAT_EQ(bar.trail(), 1.0f);
    bar.update(0.3f);
    EXPECT_FLOAT_EQ(bar.trail(), 1.0f);  // 아직 기다리는 중
    bar.update(0.05f);                   // 기다림 끝
    bar.update(0.1f);                    // 1초에 1.2씩 줄어요
    EXPECT_NEAR(bar.trail(), 1.0f - 0.12f, 1e-4f);
    bar.update(1.0f);
    EXPECT_FLOAT_EQ(bar.trail(), 0.6f);  // 진짜 바보다 짧아지지 않아요
}

TEST(DelayedBar, RiseFillsSmoothly) {
    DelayedBar bar({}, 0.0f);
    bar.set(1.0f);
    EXPECT_FLOAT_EQ(bar.value(), 0.0f);
    bar.update(0.2f);
    EXPECT_NEAR(bar.value(), 0.5f, 1e-4f);  // 1초에 2.5
    bar.update(1.0f);
    EXPECT_FLOAT_EQ(bar.value(), 1.0f);
    EXPECT_FLOAT_EQ(bar.trail(), 1.0f);
}

TEST(DelayedBar, ClampsInput) {
    DelayedBar bar;
    bar.set(-3.0f);
    EXPECT_FLOAT_EQ(bar.value(), 0.0f);
    bar.set(7.0f);
    bar.update(10.0f);
    EXPECT_FLOAT_EQ(bar.value(), 1.0f);
}

TEST(DelayedBar, AutoHideFadesWhenFullAndReturnsOnUse) {
    DelayedBarTuning tuning;
    tuning.auto_hide = true;
    DelayedBar bar(tuning);
    bar.update(1.0f);
    EXPECT_FLOAT_EQ(bar.alpha(), 1.0f);  // 아직 1.2초가 안 됐어요
    bar.update(0.5f);
    bar.update(1.0f);
    EXPECT_FLOAT_EQ(bar.alpha(), 0.3f);  // 흐려짐 (최소 0.3)
    bar.set(0.9f);
    bar.update(0.01f);
    EXPECT_FLOAT_EQ(bar.alpha(), 1.0f);  // 쓰자마자 진해짐
}

}  // namespace
