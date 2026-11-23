// 이 파일은 무엇을 하나요?
// 룬 흔적 연출(보이기·빛·되찾을 때 알갱이)을 숙제 검사(테스트)해요. (14e)
#include "game/presentation/rune_trace_view.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::RuneTraceView;

TEST(RuneTraceView, ShowsOnlyWhileThereIsATraceHere) {
    RuneTraceView view({}, nullptr);
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(16);
    view.update(0.1f, std::nullopt, {}, random, pool);
    EXPECT_FALSE(view.showing());
    EXPECT_TRUE(view.lights().empty());
    view.update(0.1f, wind::engine::Vec2{50.0f, 60.0f}, {}, random, pool);
    EXPECT_TRUE(view.showing());
    ASSERT_EQ(view.lights().size(), 1u);  // 금빛 빛 하나
    EXPECT_FLOAT_EQ(view.lights()[0].position.x, 50.0f);
}

TEST(RuneTraceView, RecoveredOrbsFlyIntoTheHero) {
    RuneTraceView view({}, nullptr);
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(16);
    view.recovered({100.0f, 100.0f}, 40, random);
    EXPECT_GT(view.flying_orbs(), 0u);
    for (int i = 0; i < 300; ++i) view.update(0.01f, std::nullopt, {100.0f, 90.0f}, random, pool);  // 3초
    EXPECT_EQ(view.flying_orbs(), 0u);  // 모두 몸에 닿아 사라져요
}

}  // namespace
