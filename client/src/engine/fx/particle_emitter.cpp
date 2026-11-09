// 이 파일은 무엇을 하나요?
// 방출기가 시간에 맞춰 점을 몇 개씩 만들지 계산하는 곳이에요.

#include "engine/fx/particle_emitter.hpp"

#include "engine/fx/particle_spawn.hpp"

#include <algorithm>

namespace wind::engine {
namespace {

// 게임이 잠깐 멈췄다 돌아왔을 때 한꺼번에 너무 많이 뿜지 않게 하는 한계 (초)
constexpr float kMaxCatchUpSeconds = 0.25f;

}  // namespace

ParticleEmitter::ParticleEmitter(const ParticleEffect& effect) : effect_(&effect) {}

std::uint32_t ParticleEmitter::update(float dt_seconds, const RectF& area, ParticleRandom& random,
                                      ParticlePool& pool) {
    const float dt = std::clamp(dt_seconds, 0.0f, kMaxCatchUpSeconds);
    pending_ += effect_->rate * dt;
    std::uint32_t added = 0;
    while (pending_ >= 1.0f) {
        pending_ -= 1.0f;
        const Vec2 position{random.range(area.x, area.right()), random.range(area.y, area.bottom())};
        if (pool.spawn(make_particle(*effect_, position, effect_->direction, random))) {
            ++added;
        }
    }
    return added;
}

}  // namespace wind::engine
