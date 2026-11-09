#pragma once

// 이 파일은 무엇을 하나요?
// 맵에 늘 떠다니는 분위기 효과(꽃잎, 반딧불 등)를 만들어요. (명세 6.2)
// 어떤 효과를 쓸지는 맵 파일의 "ambience.ambient_fx" 목록이 정해요.
// 점은 카메라가 보는 곳 근처에서만 만들어요. 안 보이는 먼 곳까지 만들면 낭비니까요.

#include "engine/core/math.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_emitter.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"

#include <cstddef>
#include <vector>

namespace wind::game {

class AmbientParticles {
public:
    // 효과 하나를 계속 뿜도록 더해요. 효과는 이 물건보다 오래 살아 있어야 해요.
    void add(const engine::ParticleEffect& effect);

    // 시간이 흘렀어요. view: 카메라가 보는 맵 영역. 그보다 조금 넓은 곳에서 점을 만들어요
    // (화면 가장자리 바로 밖에서 생겨야 들어올 때 "뿅" 하고 나타나지 않아요).
    void update(float dt_seconds, const engine::RectF& view, engine::ParticleRandom& random,
                engine::ParticlePool& pool);

    [[nodiscard]] std::size_t count() const { return emitters_.size(); }

private:
    std::vector<engine::ParticleEmitter> emitters_;
};

}  // namespace wind::game
