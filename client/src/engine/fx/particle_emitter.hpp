#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 방출기 = 효과 하나를 "계속" 뿜어 주는 분수예요.
// 1초에 rate개씩, 정해진 네모 영역 안 아무 곳에서나 점을 만들어요.
// 예: 화면 안 아무 곳에서나 1초에 4개씩 꽃잎이 생겨나기
// 0.3개처럼 딱 떨어지지 않는 수는 모아 뒀다가 1개가 되면 만들어요.

#include "engine/core/math.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"

#include <cstdint>

namespace wind::engine {

class ParticleEmitter {
public:
    // effect는 방출기보다 오래 살아 있어야 해요 (방출기는 빌려 보기만 해요).
    explicit ParticleEmitter(const ParticleEffect& effect);

    // 시간이 dt초 흘렀어요. area 안에서 점을 만들어 통에 넣고, 만든 개수를 돌려줘요.
    std::uint32_t update(float dt_seconds, const RectF& area, ParticleRandom& random,
                         ParticlePool& pool);

    [[nodiscard]] const ParticleEffect& effect() const { return *effect_; }

private:
    const ParticleEffect* effect_;
    float pending_ = 0.0f;  // 아직 만들지 못한 몫 (0.3개 같은 것)
};

}  // namespace wind::engine
