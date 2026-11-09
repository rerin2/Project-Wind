// 이 파일은 무엇을 하나요?
// 카메라 앞보기(LookAhead)가 가는 쪽으로 부드럽게 움직이는지 숙제 검사(테스트)를 해요.
#include "engine/camera/look_ahead.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::LookAhead;

// 오른쪽으로 계속 가면 앞보기 거리가 최대 거리에 가까워지고, 넘지는 않아요.
TEST(LookAhead, ConvergesTowardDirection) {
    LookAhead look(28.0f, 3.0f);
    look.update({1.0f, 0.0f}, 1.0f / 60.0f);
    EXPECT_GT(look.offset().x, 0.0f);
    EXPECT_LT(look.offset().x, 5.0f);  // 첫 장에서 휙 가지 않아요
    for (int i = 0; i < 600; ++i) {
        look.update({1.0f, 0.0f}, 1.0f / 60.0f);
    }
    EXPECT_NEAR(look.offset().x, 28.0f, 0.1f);
    EXPECT_LE(look.offset().x, 28.0f);
}

// 멈추면 다시 가운데(0)로 돌아와요.
TEST(LookAhead, ReturnsToCenterWhenStill) {
    LookAhead look(28.0f, 3.0f);
    for (int i = 0; i < 600; ++i) {
        look.update({0.0f, -1.0f}, 1.0f / 60.0f);
    }
    for (int i = 0; i < 600; ++i) {
        look.update({0.0f, 0.0f}, 1.0f / 60.0f);
    }
    EXPECT_NEAR(look.offset().y, 0.0f, 0.1f);
}

}  // namespace
