#pragma once

// 이 파일은 무엇을 하나요?
// 걸을 때 발밑에서 "폴폴" 피어오르는 먼지와, 구를 때 "팍" 튀는 먼지를 만들어요. (명세 6.2)
// 먼지가 "언제" 나는지는 여기서 정하고, 먼지가 "어떻게 생겼는지"는 효과 파일이 정해요.
//   걷기 먼지: content/fx/dust_walk.json, 구르기 먼지: content/fx/dust_dodge.json
// 보여 주기만 하는 연출이라 게임 규칙에는 아무 영향이 없어요.

#include "engine/core/math.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"

namespace wind::game {

class DustEmitter {
public:
    // 효과가 없으면(nullptr) 그 먼지는 만들지 않아요. 효과는 이 물건보다 오래 살아 있어야 해요.
    DustEmitter(const engine::ParticleEffect* walk, const engine::ParticleEffect* dodge);

    // 걷는 중이에요: 일정 거리를 걸을 때마다 걷기 먼지를 한 번 터뜨려요.
    // feet: 발 위치, velocity: 지금 속도
    void walk(engine::Vec2 feet, engine::Vec2 velocity, float dt_seconds,
              engine::ParticleRandom& random, engine::ParticlePool& pool);
    // 구르기를 시작했어요: 반대쪽으로 구르기 먼지를 확 뿌려요.
    void dodge_burst(engine::Vec2 feet, engine::Vec2 direction, engine::ParticleRandom& random,
                     engine::ParticlePool& pool);

private:
    const engine::ParticleEffect* walk_;
    const engine::ParticleEffect* dodge_;
    float distance_since_puff_ = 0.0f;  // 마지막 먼지 뒤로 걸은 거리
};

}  // namespace wind::game
