#pragma once

// 이 파일은 무엇을 하나요?
// 파티클을 담아 두는 "통"이에요. 점들을 움직이고, 수명이 다 된 점은 치워요.
// 통의 크기(capacity)가 정해져 있어서, 꽉 차면 새 점을 버리고 몇 개 버렸는지 세요.
// 점이 끝없이 늘어나 게임이 느려지는 걸 막는 안전장치예요. (명세 6.6)

#include "engine/fx/particle.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace wind::engine {

class ParticlePool {
public:
    explicit ParticlePool(std::size_t capacity);

    // 새 점 하나를 넣어요. 통이 꽉 찼으면 넣지 않고 false를 돌려줘요.
    bool spawn(const Particle& particle);
    // 시간이 dt초 흘렀어요: 모든 점을 움직이고, 수명이 다 된 점을 치워요.
    void update(float dt_seconds);
    // 모든 점을 치워요 (맵을 바꿀 때 등).
    void clear() { particles_.clear(); }

    [[nodiscard]] const std::vector<Particle>& particles() const { return particles_; }
    [[nodiscard]] std::size_t size() const { return particles_.size(); }
    [[nodiscard]] std::size_t capacity() const { return capacity_; }
    // 통이 꽉 차서 버린 점의 수 (지금까지 모두)
    [[nodiscard]] std::uint64_t dropped() const { return dropped_; }

private:
    std::size_t capacity_;
    std::vector<Particle> particles_;
    std::uint64_t dropped_ = 0;
};

}  // namespace wind::engine
