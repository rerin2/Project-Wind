// 이 파일은 무엇을 하나요?
// 화면 전환 페이드가 밝음 -> 어두워지기 -> 까맘 -> 밝아지기 -> 밝음 순서로 가는지 숙제 검사해요.
#include "engine/fx/screen_fade.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::ScreenFade;

TEST(ScreenFade, FullCycle) {
    ScreenFade fade(0.4f);
    EXPECT_EQ(fade.phase(), ScreenFade::Phase::Clear);
    EXPECT_FALSE(fade.busy());
    EXPECT_FLOAT_EQ(fade.darkness(), 0.0f);

    fade.fade_out();
    EXPECT_TRUE(fade.busy());
    fade.update(0.2f);
    EXPECT_GT(fade.darkness(), 0.0f);
    EXPECT_LT(fade.darkness(), 1.0f);
    fade.update(0.2f);
    // 다 까매지면 "덮임" 상태로 기다려요 (그동안 새 맵을 불러와요).
    EXPECT_EQ(fade.phase(), ScreenFade::Phase::Covered);
    EXPECT_FLOAT_EQ(fade.darkness(), 1.0f);
    fade.update(5.0f);
    EXPECT_EQ(fade.phase(), ScreenFade::Phase::Covered);

    fade.fade_in();
    fade.update(0.2f);
    EXPECT_GT(fade.darkness(), 0.0f);
    EXPECT_LT(fade.darkness(), 1.0f);
    fade.update(0.2f);
    EXPECT_EQ(fade.phase(), ScreenFade::Phase::Clear);
    EXPECT_FALSE(fade.busy());
}

// 어두워지는 도중에 밝아지기를 시작하면, 지금 어두운 정도에서 이어서 밝아져요 (갑자기 튀지 않게).
TEST(ScreenFade, ReverseMidwayContinuesFromCurrentDarkness) {
    ScreenFade fade(0.4f);
    fade.fade_out();
    fade.update(0.3f);
    const float mid = fade.darkness();
    fade.fade_in();
    EXPECT_FLOAT_EQ(fade.darkness(), mid);
}

}  // namespace
