// 이 파일은 무엇을 하나요?
// 저장 아이콘의 시간 흐름(16c)을 숙제 검사(테스트)해요: 켜자마자는 안 뜸, 저장하면 나타남,
// 잠깐 보였다 사라짐, 보이는 동안 또 저장하면 다시 늘어남.
#include "game/ui/save_icon.hpp"

#include <gtest/gtest.h>

using wind::game::SaveIcon;

namespace {

// seconds 동안 1/60초씩 흘려요 (저장 횟수는 그대로)
void run(SaveIcon& icon, float seconds, unsigned saves) {
    for (float t = 0.0f; t < seconds; t += 1.0f / 60.0f) icon.update(1.0f / 60.0f, saves);
}

}  // namespace

TEST(SaveIcon, FirstCountIsOnlyTheStartingPoint) {
    SaveIcon icon;
    icon.update(0.016f, 3);  // 불러온 게임처럼 처음부터 3이어도
    run(icon, 0.5f, 3);
    EXPECT_FALSE(icon.visible());
}

TEST(SaveIcon, ShowsAfterASaveThenFades) {
    SaveIcon icon;
    icon.update(0.016f, 0);
    icon.update(0.016f, 1);                 // 저장!
    run(icon, SaveIcon::kFadeInSeconds + 0.05f, 1);
    EXPECT_FLOAT_EQ(icon.alpha(), 1.0f);    // 다 나타남
    run(icon, SaveIcon::kHoldSeconds - SaveIcon::kFadeInSeconds, 1);
    EXPECT_LT(icon.alpha(), 1.0f);          // 사라지는 중
    EXPECT_TRUE(icon.visible());
    run(icon, SaveIcon::kFadeOutSeconds + 0.05f, 1);
    EXPECT_FALSE(icon.visible());           // 다 사라짐
}

TEST(SaveIcon, AnotherSaveWhileShownKeepsItLonger) {
    SaveIcon icon;
    icon.update(0.016f, 0);
    icon.update(0.016f, 1);
    run(icon, 1.0f, 1);
    const float spun = icon.shown_seconds();
    icon.update(0.016f, 3);                 // 쓰러짐처럼 두 번 한꺼번에 (본+자동)
    EXPECT_FLOAT_EQ(icon.alpha(), 1.0f);    // 다시 나타나지 않고 그대로 진해요
    run(icon, 1.0f, 3);                     // 처음 저장에서 2초 = 원래라면 사라졌을 때
    EXPECT_FLOAT_EQ(icon.alpha(), 1.0f);    // 마지막 저장 뒤 1.2초 전이라 그대로
    EXPECT_GT(icon.shown_seconds(), spun);  // 반짝이는 이어서 돌아요
}
