// 이 파일은 무엇을 하나요?
// 맵 분위기 효과가 카메라 근처(화면보다 조금 넓은 곳)에서만 생기는지 숙제 검사(테스트)를 해요.
#include "game/presentation/ambient_particles.hpp"

#include <gtest/gtest.h>

namespace {

TEST(AmbientParticles, SpawnsAroundView) {
    wind::engine::ParticleEffect effect;
    effect.lifetime = {5.0f, 5.0f};
    effect.rate = 200.0f;
    wind::game::AmbientParticles ambient;
    ambient.add(effect);
    EXPECT_EQ(ambient.count(), 1u);

    wind::engine::ParticleRandom random(9);
    wind::engine::ParticlePool pool(500);
    const wind::engine::RectF view{1000.0f, 2000.0f, 640.0f, 360.0f};
    ambient.update(0.2f, view, random, pool);
    ASSERT_GT(pool.size(), 0u);
    for (const auto& p : pool.particles()) {
        // 화면보다 48픽셀 넓은 곳까지만
        EXPECT_GE(p.position.x, view.x - 48.0f);
        EXPECT_LE(p.position.x, view.right() + 48.0f);
        EXPECT_GE(p.position.y, view.y - 48.0f);
        EXPECT_LE(p.position.y, view.bottom() + 48.0f);
    }
}

}  // namespace
