// 이 파일은 무엇을 하나요?
// 피격 반응(HitReaction)을 숙제 검사(테스트)해요. (11c)
// - 경직은 0.3초, 무적은 0.6초 이어져요
// - 넉백은 맞은 쪽으로 밀리고, 다 합치면 빠르기 × 경직 ÷ 2 (160px/s × 0.3초 ÷ 2 = 24픽셀)
// - 시간 0(히트스톱)에는 아무것도 줄지 않아요
#include "game/entities/hit_reaction.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::HitReaction;
using wind::game::HitTuning;

constexpr float kDt = 1.0f / 60.0f;

TEST(HitReaction, StaggerAndInvulnerabilityLast) {
    HitReaction reaction(HitTuning{});
    EXPECT_FALSE(reaction.staggered());
    EXPECT_FALSE(reaction.invulnerable());
    reaction.start({1.0f, 0.0f}, 160.0f);
    EXPECT_TRUE(reaction.staggered());
    EXPECT_TRUE(reaction.invulnerable());
    for (int i = 0; i < 17; ++i) (void)reaction.step(kDt);  // 0.283초
    EXPECT_TRUE(reaction.staggered());
    for (int i = 0; i < 2; ++i) (void)reaction.step(kDt);   // 0.317초: 경직 끝
    EXPECT_FALSE(reaction.staggered());
    EXPECT_TRUE(reaction.invulnerable());                    // 무적은 아직
    for (int i = 0; i < 18; ++i) (void)reaction.step(kDt);  // 0.617초: 무적 끝
    EXPECT_FALSE(reaction.invulnerable());
}

TEST(HitReaction, KnockbackTotalsHalfSpeedTimesStagger) {
    HitReaction reaction(HitTuning{});
    reaction.start({0.0f, 2.0f}, 160.0f);  // 방향은 길이 1로 바꿔 써요 (아래쪽)
    wind::engine::Vec2 total;
    float first = 0.0f;
    float last = 0.0f;
    for (int i = 0; i < 30; ++i) {
        const auto step = reaction.step(kDt);
        if (i == 0) first = step.y;
        if (step.y > 0.0f) last = step.y;
        total = total + step;
    }
    EXPECT_NEAR(total.x, 0.0f, 1e-4f);
    EXPECT_NEAR(total.y, 24.0f, 0.01f);  // 160 × 0.3 ÷ 2
    EXPECT_GT(first, last);               // 처음엔 빠르고 끝엔 느려요
}

TEST(HitReaction, ZeroTimeChangesNothingAndBadValuesAreSafe) {
    HitReaction reaction(HitTuning{});
    reaction.start({1.0f, 0.0f}, 160.0f);
    const auto step = reaction.step(0.0f);  // 히트스톱
    EXPECT_FLOAT_EQ(step.x, 0.0f);
    EXPECT_TRUE(reaction.staggered());
    EXPECT_FLOAT_EQ(reaction.invulnerable_left(), 0.6f);

    HitReaction odd(HitTuning{});
    odd.start({0.0f, 0.0f}, -50.0f);  // 방향 0, 음수 힘: 밀리지 않지만 경직·무적은 돼요
    EXPECT_FLOAT_EQ(wind::engine::length(odd.step(kDt)), 0.0f);
    EXPECT_TRUE(odd.staggered());
}

}  // namespace
