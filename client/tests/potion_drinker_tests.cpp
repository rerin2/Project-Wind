// 이 파일은 무엇을 하나요?
// 물약 마시기 시간표(0.6초, 0.35초 꿀꺽, 끊기기)를 숙제 검사(테스트)해요. (14d)
#include "game/entities/potion_drinker.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::PotionDrinker;

TEST(PotionDrinker, GulpsOnceThenFinishes) {
    PotionDrinker drinker;
    drinker.start({50, 0});
    EXPECT_TRUE(drinker.busy());
    EXPECT_FALSE(drinker.step(0.3f).has_value());  // 0.3초: 아직 꿀꺽 전
    const auto effect = drinker.step(0.1f);         // 0.4초: 꿀꺽
    ASSERT_TRUE(effect.has_value());
    EXPECT_EQ(effect->heal_hp, 50);
    EXPECT_TRUE(drinker.gulped());
    EXPECT_FALSE(drinker.step(0.1f).has_value());  // 꿀꺽은 한 번만
    EXPECT_TRUE(drinker.busy());                    // 0.5초: 병 내리는 중
    (void)drinker.step(0.11f);                      // 0.61초: 끝
    EXPECT_FALSE(drinker.busy());
}

TEST(PotionDrinker, LongStepStillGulps) {
    PotionDrinker drinker;
    drinker.start({0, 30});
    const auto effect = drinker.step(1.0f);  // 한 번에 끝까지 가도 꿀꺽은 빠지지 않아요
    ASSERT_TRUE(effect.has_value());
    EXPECT_EQ(effect->heal_mp, 30);
    EXPECT_FALSE(drinker.busy());
}

TEST(PotionDrinker, CancelBeforeGulpLosesTheEffect) {
    PotionDrinker drinker;
    drinker.start({50, 0});
    (void)drinker.step(0.2f);
    drinker.cancel();  // 맞았어요
    EXPECT_FALSE(drinker.busy());
    EXPECT_FALSE(drinker.step(1.0f).has_value());  // 효과는 사라졌어요
}

TEST(PotionDrinker, IdleDrinkerDoesNothing) {
    PotionDrinker drinker;
    EXPECT_FALSE(drinker.busy());
    EXPECT_FALSE(drinker.step(1.0f).has_value());
}

}  // namespace
