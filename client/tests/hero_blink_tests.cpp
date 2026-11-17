// 이 파일은 무엇을 하나요?
// 무적 깜빡임(hero_blink_alpha)을 숙제 검사(테스트)해요. (11c)
// - 무적 동안만 주인공이 흐려졌다 돌아오고, 완전히 사라지지는 않아요. 무적이 끝나면 늘 보여요
#include "game/presentation/hero_blink.hpp"

#include <gtest/gtest.h>

namespace {

TEST(HeroBlink, BlinksOnlyWhileInvulnerable) {
    wind::game::HitReaction reaction(wind::game::HitTuning{});
    EXPECT_FLOAT_EQ(wind::game::hero_blink_alpha(reaction), 1.0f);  // 안 맞았으면 늘 보여요
    reaction.start({1.0f, 0.0f}, 0.0f);
    bool dimmed = false;
    bool shown = false;
    for (int i = 0; i < 36; ++i) {  // 무적 0.6초 동안 흐림과 보임이 번갈아 나와요
        const float alpha = wind::game::hero_blink_alpha(reaction);
        dimmed = dimmed || alpha < 1.0f;
        shown = shown || alpha == 1.0f;
        EXPECT_GT(alpha, 0.0f);  // 완전히 사라지지는 않아요
        (void)reaction.step(1.0f / 60.0f);
    }
    EXPECT_TRUE(dimmed);
    EXPECT_TRUE(shown);
    for (int i = 0; i < 5; ++i) (void)reaction.step(1.0f / 60.0f);
    EXPECT_FLOAT_EQ(wind::game::hero_blink_alpha(reaction), 1.0f);  // 무적이 끝나면 그대로
}

}  // namespace
