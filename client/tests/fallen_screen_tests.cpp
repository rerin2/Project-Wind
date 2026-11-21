// 이 파일은 무엇을 하나요?
// "쓰러졌다" 화면 시간표(FallenScreen)를 숙제 검사해요. (12b)
// - 쓰러진 뒤 1.2초까지는 안 보여요 (슬로모션·흑백이 먼저)
// - 1.2~2.0초 동안 나타나며 아래에서 올라와요, 2.0초부터 부활 가능(ready) + 안내
// - "처음 화면"(쓰러지기 전)과 다시 선 뒤에는 아무것도 안 보이고 ready도 아니에요
#include "game/ui/fallen_screen.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::FallenScreen;

TEST(FallenScreen, HiddenWhileStanding) {
    FallenScreen screen;
    EXPECT_FALSE(screen.visible());
    EXPECT_FALSE(screen.ready());
    screen.update(1.0f, false);
    EXPECT_FALSE(screen.visible());
    EXPECT_FLOAT_EQ(screen.hint_alpha(), 0.0f);
}

TEST(FallenScreen, WaitsThenFadesInThenReady) {
    FallenScreen screen;
    screen.update(1.1f, true);  // 아직 기다리는 중
    EXPECT_FALSE(screen.visible());
    EXPECT_FALSE(screen.ready());
    EXPECT_FLOAT_EQ(screen.drop(), FallenScreen::kDropPixels);

    screen.update(0.5f, true);  // 1.6초: 나타나는 중
    EXPECT_TRUE(screen.visible());
    EXPECT_GT(screen.alpha(), 0.0f);
    EXPECT_LT(screen.alpha(), 1.0f);
    EXPECT_GT(screen.drop(), 0.0f);
    EXPECT_FALSE(screen.ready());
    EXPECT_FLOAT_EQ(screen.hint_alpha(), 0.0f);  // 안내는 아직

    screen.update(0.41f, true);  // 2.01초: 다 나타남 (경계 조금 넘게)
    EXPECT_FLOAT_EQ(screen.alpha(), 1.0f);
    EXPECT_FLOAT_EQ(screen.drop(), 0.0f);
    EXPECT_TRUE(screen.ready());
    EXPECT_GT(screen.hint_alpha(), 0.0f);

    screen.update(1.0f, true);
    EXPECT_FLOAT_EQ(screen.hint_alpha(), 1.0f);
}

TEST(FallenScreen, StandingUpHidesAtOnce) {
    FallenScreen screen;
    screen.update(3.0f, true);
    ASSERT_TRUE(screen.ready());
    screen.update(0.016f, false);  // 새 맵에서 다시 섬
    EXPECT_FALSE(screen.visible());
    EXPECT_FALSE(screen.ready());
}

}  // namespace
