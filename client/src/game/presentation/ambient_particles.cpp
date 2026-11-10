// 이 파일은 무엇을 하나요?
// 분위기 효과 방출기들을 카메라 근처에서 돌리는 곳이에요.

#include "game/presentation/ambient_particles.hpp"

namespace wind::game {
namespace {

// 화면 바깥으로 얼마나 더 넓게 점을 만들지 (픽셀)
constexpr float kViewMargin = 48.0f;

}  // namespace

void AmbientParticles::add(const engine::ParticleEffect& effect) {
    emitters_.emplace_back(effect);
}

void AmbientParticles::update(float dt_seconds, const engine::RectF& view,
                              engine::ParticleRandom& random, engine::ParticlePool& pool,
                              float rate_scale) {
    const engine::RectF area{view.x - kViewMargin, view.y - kViewMargin,
                             view.w + kViewMargin * 2.0f, view.h + kViewMargin * 2.0f};
    for (auto& emitter : emitters_) {
        emitter.update(dt_seconds, area, random, pool, rate_scale);
    }
}

}  // namespace wind::game
