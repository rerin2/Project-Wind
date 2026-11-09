#pragma once

// 이 파일은 무엇을 하나요?
// 효과 요리법(ParticleEffect)을 보고 점(Particle)을 실제로 만드는 도우미 함수들이에요.

#include "engine/core/math.hpp"
#include "engine/fx/particle.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"

#include <cstdint>

namespace wind::engine {

// 점 하나를 만들어요. direction_degrees: 나가는 방향 (보통 effect.direction)
[[nodiscard]] Particle make_particle(const ParticleEffect& effect, Vec2 position,
                                     float direction_degrees, ParticleRandom& random);

// effect.burst개를 한꺼번에 만들어 통에 넣어요. 통에 실제로 들어간 개수를 돌려줘요.
std::uint32_t emit_burst(const ParticleEffect& effect, Vec2 position, float direction_degrees,
                         ParticleRandom& random, ParticlePool& pool);

// 화살표(Vec2) 방향을 각도(도)로 바꿔요. 0 = 오른쪽, 90 = 아래
[[nodiscard]] float direction_to_degrees(Vec2 direction);

}  // namespace wind::engine
