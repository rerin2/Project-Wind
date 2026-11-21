// 이 파일은 무엇을 하나요?
// 부활 시점(RespawnTrigger)을 숙제 검사해요. (12c)
// - 서 있거나 쓰러짐 화면이 아직 다 안 나타났으면 Enter를 눌러도 부활하지 않아요
// - 화면이 다 나타난 뒤 Enter면 바로, 안 누르면 2.5초 뒤 저절로 한 번
// - 맵을 못 열어 아직 쓰러져 있으면 2.5초 뒤 다시 알려요
#include "game/world/respawn_trigger.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::RespawnTrigger;

constexpr float kDt = 1.0f / 60.0f;

TEST(RespawnTrigger, NothingWhileStandingOrScreenNotReady) {
    RespawnTrigger trigger;
    EXPECT_FALSE(trigger.update(10.0f, false, true, true));  // 서 있음
    EXPECT_FALSE(trigger.update(10.0f, true, false, true));  // 쓰러짐 화면이 아직 나타나는 중: Enter 무시
    EXPECT_FLOAT_EQ(trigger.auto_left(), RespawnTrigger::kAutoSeconds);
}

TEST(RespawnTrigger, EnterRespawnsAtOnce) {
    RespawnTrigger trigger;
    EXPECT_FALSE(trigger.update(kDt, true, true, false));
    EXPECT_TRUE(trigger.update(kDt, true, true, true));   // Enter: 바로
    EXPECT_FALSE(trigger.update(kDt, true, true, false)); // 한 번만
}

TEST(RespawnTrigger, AutoRespawnsAfterTwoAndAHalfSeconds) {
    RespawnTrigger trigger;
    int fired_at = -1;
    for (int i = 0; i < 300 && fired_at < 0; ++i) {
        if (trigger.update(kDt, true, true, false)) fired_at = i + 1;
    }
    EXPECT_NEAR(fired_at, 150, 1);  // 2.5초 = 150박자 (소수 오차로 한 박자 차이는 괜찮아요)
}

TEST(RespawnTrigger, RetriesIfStillDownAndResetsWhenStanding) {
    RespawnTrigger trigger;
    EXPECT_TRUE(trigger.update(kDt, true, true, true));
    EXPECT_FALSE(trigger.update(2.0f, true, true, false));  // 아직 쓰러져 있음(맵을 못 엶): 기다려요
    EXPECT_TRUE(trigger.update(0.6f, true, true, false));   // 2.5초 지나 다시 알려요
    (void)trigger.update(1.0f, true, true, false);
    EXPECT_FALSE(trigger.update(kDt, false, false, false)); // 일어나면 처음으로
    EXPECT_FLOAT_EQ(trigger.auto_left(), RespawnTrigger::kAutoSeconds);
}

}  // namespace
