// 이 파일은 무엇을 하나요?
// 게임 시계가 맞게 가는지 숙제 검사(테스트)를 해요: 실제 60초 = 게임 1시간, 자정 넘기, 시각 글자.
#include "game/world/game_clock.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::GameClock;

TEST(GameClock, AdvancesAtConfiguredSpeed) {
    GameClock clock(9.0f, 60.0f);
    clock.advance(90.0);  // 실제 90초 = 게임 1시간 30분
    EXPECT_FLOAT_EQ(clock.hour(), 10.5f);
    EXPECT_EQ(clock.text(), "10:30");
}

// 23시에서 2시간 가면 다음 날 1시
TEST(GameClock, WrapsToNextDay) {
    GameClock clock(23.0f, 60.0f);
    clock.skip_hours(2.0f);
    EXPECT_NEAR(clock.hour(), 1.0f, 1e-4f);
    EXPECT_EQ(clock.day(), 1);
}

// 거꾸로(음수) 가지 않아요.
TEST(GameClock, IgnoresNegativeTime) {
    GameClock clock(9.0f, 60.0f);
    clock.advance(-100.0);
    clock.skip_hours(-3.0f);
    EXPECT_FLOAT_EQ(clock.hour(), 9.0f);
}

}  // namespace

// 저장에서 되살리기 (16b): 날짜와 시각을 그대로, 범위 밖은 맞춰요
TEST(GameClock, SetRestoresDayAndHour) {
    GameClock clock(9.0f, 60.0f);
    clock.set(3, 18.5f);
    EXPECT_EQ(clock.day(), 3);
    EXPECT_FLOAT_EQ(clock.hour(), 18.5f);
    clock.set(-2, 30.0f);
    EXPECT_EQ(clock.day(), 0);
    EXPECT_LT(clock.hour(), 24.0f);
}
