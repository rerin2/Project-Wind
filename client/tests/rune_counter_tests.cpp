// 이 파일은 무엇을 하나요?
// HUD 룬 숫자 검사 (13c): 처음 값은 바로, 늘면 굴러 올라가며 "+N", 줄면 빨갛게 내려가며 "-N".
#include "game/ui/rune_counter.hpp"

#include <gtest/gtest.h>

using wind::game::RuneCounter;

namespace {
constexpr float kDt = 1.0f / 60.0f;
}

TEST(RuneCounter, FirstValueShowsAtOnce) {
    RuneCounter counter;
    counter.update(kDt, 500);
    EXPECT_EQ(counter.shown(), 500);
    EXPECT_EQ(counter.popup(), 0);  // 처음 값은 "+500"을 띄우지 않아요
}

TEST(RuneCounter, GainRollsUpWithPopup) {
    RuneCounter counter;
    counter.update(kDt, 0);
    counter.update(kDt, 12);
    EXPECT_EQ(counter.popup(), 12);
    EXPECT_EQ(counter.shown(), 0);      // 막 굴리기 시작
    counter.update(0.2f, 12);
    EXPECT_GT(counter.shown(), 0);      // 중간
    EXPECT_LT(counter.shown(), 12);
    counter.update(0.5f, 12);
    EXPECT_EQ(counter.shown(), 12);     // 0.6초 뒤 딱 맞아요
    counter.update(0.1f, 24);           // 또 잡으면 떠 있는 숫자에 더해요
    EXPECT_EQ(counter.popup(), 24);
    counter.update(2.0f, 24);
    EXPECT_EQ(counter.popup(), 0);      // 1.2초 뒤 사라져요
}

TEST(RuneCounter, LossRollsDownRed) {
    RuneCounter counter;
    counter.update(kDt, 25);
    counter.update(kDt, 13);            // 쓰러져서 절반 잃음
    EXPECT_TRUE(counter.losing());
    EXPECT_EQ(counter.popup(), -12);
    counter.update(1.0f, 13);
    EXPECT_FALSE(counter.losing());
    EXPECT_EQ(counter.shown(), 13);
}
