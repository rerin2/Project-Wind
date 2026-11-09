// 이 파일은 무엇을 하나요?
// 파티클 주사위를 굴리는 곳이에요.

#include "engine/fx/particle_random.hpp"

#include <algorithm>

namespace wind::engine {

ParticleRandom::ParticleRandom(std::uint32_t seed) : engine_(seed) {}

float ParticleRandom::range(float a, float b) {
    if (a == b) {
        return a;
    }
    std::uniform_real_distribution<float> dist(std::min(a, b), std::max(a, b));
    return dist(engine_);
}

}  // namespace wind::engine
