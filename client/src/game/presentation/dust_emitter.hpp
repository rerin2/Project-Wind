#pragma once

// 이 파일은 무엇을 하나요?
// 걸을 때 발밑에서 "폴폴" 피어오르는 먼지를 만들어요. (명세 6.2)
// 구르기를 시작하면 먼지가 "팍" 하고 한꺼번에 튀어요.
// 보여 주기만 하는 연출이라 게임 규칙에는 아무 영향이 없어요.

#include "engine/core/math.hpp"
#include "engine/fx/particle_pool.hpp"

#include <cstdint>
#include <random>

namespace wind::game {

class DustEmitter {
public:
    explicit DustEmitter(std::uint32_t seed);

    // 걷는 중이에요: 일정 거리를 걸을 때마다 먼지 한 줌을 만들어요.
    // feet: 발 위치, velocity: 지금 속도
    void walk(engine::Vec2 feet, engine::Vec2 velocity, float dt_seconds,
              engine::ParticlePool& pool);
    // 구르기를 시작했어요: 반대쪽으로 먼지를 확 뿌려요.
    void dodge_burst(engine::Vec2 feet, engine::Vec2 direction, engine::ParticlePool& pool);

private:
    // a ~ b 사이의 무작위 숫자
    float random(float a, float b);
    void puff(engine::Vec2 feet, engine::Vec2 push, engine::ParticlePool& pool);

    std::minstd_rand rng_;
    float distance_since_puff_ = 0.0f;  // 마지막 먼지 뒤로 걸은 거리
};

}  // namespace wind::game
