// 이 파일은 무엇을 하나요?
// 움직이는 그림 플레이어(Animator)가 맞게 그림을 넘기는지 숙제 검사(테스트)를 해요.
// - 시간이 흐르면 다음 그림으로 넘어가나, 끝나면 처음으로 돌아가나(반복), 멈추나
#include "engine/animation/animator.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::AnimationClip;
using wind::engine::Animator;

// 검사용 클립: 그림 판의 4, 5, 6번 칸을 0.1초씩 보여 줘요.
AnimationClip make_clip(bool loop) {
    return {{4, 5, 6}, {0.1, 0.1, 0.1}, loop};
}

// 0.05초 뒤엔 아직 첫 그림(4), 0.1초가 넘으면 둘째 그림(5), 0.2초가 넘으면 셋째 그림(6)
TEST(Animator, AdvancesFramesOverTime) {
    const auto clip = make_clip(true);
    Animator animator;
    animator.play(&clip);
    EXPECT_EQ(animator.column(), 4);
    animator.update(0.05);
    EXPECT_EQ(animator.column(), 4);
    animator.update(0.06);
    EXPECT_EQ(animator.column(), 5);
    animator.update(0.1);
    EXPECT_EQ(animator.column(), 6);
}

// 반복 클립은 세 장(0.3초)을 다 보여 주면 다시 첫 그림으로 돌아가야 해요.
TEST(Animator, LoopsBackToFirstFrame) {
    const auto clip = make_clip(true);
    Animator animator;
    animator.play(&clip);
    animator.update(0.31);
    EXPECT_EQ(animator.column(), 4);
    EXPECT_FALSE(animator.finished());
}

// 반복하지 않는 클립은 마지막 그림에서 멈추고 "끝났어요"(finished)가 돼야 해요.
TEST(Animator, NonLoopingClipStopsOnLastFrame) {
    const auto clip = make_clip(false);
    Animator animator;
    animator.play(&clip);
    animator.update(5.0);
    EXPECT_EQ(animator.column(), 6);
    EXPECT_TRUE(animator.finished());
}

// 같은 클립을 또 틀어도 하던 곳부터 계속해야 해요. "처음부터"(restart)를 부탁하면 처음으로.
TEST(Animator, ReplayingSameClipKeepsPosition) {
    const auto clip = make_clip(true);
    Animator animator;
    animator.play(&clip);
    animator.update(0.15);
    animator.play(&clip);
    EXPECT_EQ(animator.column(), 5);
    animator.play(&clip, /*restart=*/true);
    EXPECT_EQ(animator.column(), 4);
}

}  // namespace
