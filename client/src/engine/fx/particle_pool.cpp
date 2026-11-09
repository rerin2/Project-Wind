// 이 파일은 무엇을 하나요?
// 파티클 통이 점들을 움직이고 치우는 방법이에요.

#include "engine/fx/particle_pool.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {

ParticlePool::ParticlePool(std::size_t capacity) : capacity_(capacity) {
    particles_.reserve(capacity_);  // 미리 자리를 잡아 두면 중간에 통을 다시 만들 일이 없어요
}

bool ParticlePool::spawn(const Particle& particle) {
    if (particles_.size() >= capacity_) {
        ++dropped_;  // 꽉 찼어요. 버리고 세기만 해요.
        return false;
    }
    particles_.push_back(particle);
    return true;
}

void ParticlePool::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    for (Particle& p : particles_) {
        p.age += dt_seconds;
        // 공기 저항: 1초마다 속도가 e^(-drag)배로 줄어요. 화면 장 수와 상관없이 같은 느낌이에요.
        p.velocity = p.velocity * std::exp(-p.drag * dt_seconds);
        p.velocity.y += p.gravity * dt_seconds;  // 중력은 아래(+y)로 당겨요
        p.position = p.position + p.velocity * dt_seconds;
    }
    // 수명이 다 된 점들을 한 번에 치워요.
    std::erase_if(particles_, [](const Particle& p) { return p.age >= p.lifetime; });
}

}  // namespace wind::engine
