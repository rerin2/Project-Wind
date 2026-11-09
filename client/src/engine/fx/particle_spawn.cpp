// 이 파일은 무엇을 하나요?
// 요리법의 "범위"(최소~최대)에서 주사위를 굴려 점 하나하나를 조금씩 다르게 만들어요.

#include "engine/fx/particle_spawn.hpp"

#include <cmath>
#include <numbers>

namespace wind::engine {
namespace {

constexpr float kDegToRad = std::numbers::pi_v<float> / 180.0f;

}  // namespace

Particle make_particle(const ParticleEffect& effect, Vec2 position, float direction_degrees,
                       ParticleRandom& random) {
    Particle p;
    // 생기는 자리를 중심 근처로 살짝 흩어요.
    p.position = {position.x + random.range(-effect.spawn_radius, effect.spawn_radius),
                  position.y + random.range(-effect.spawn_radius, effect.spawn_radius)};
    // 방향은 기준 방향에서 흩어짐 폭의 절반만큼 왼쪽~오른쪽으로 흔들어요.
    const float half_spread = effect.spread * 0.5f;
    const float angle = (direction_degrees + random.range(-half_spread, half_spread)) * kDegToRad;
    const float speed = random.range(effect.speed.min, effect.speed.max);
    p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
    p.drag = effect.drag;
    p.gravity = effect.gravity;
    p.lifetime = random.range(effect.lifetime.min, effect.lifetime.max);
    p.start_size = random.range(effect.start_size.min, effect.start_size.max);
    p.end_size = effect.end_size;
    p.start_color = effect.start_color;
    p.end_color = effect.end_color;
    p.fade_in = effect.fade_in;
    p.fade_out = effect.fade_out;
    p.wobble_amplitude = effect.wobble_amplitude;
    p.wobble_frequency = effect.wobble_frequency;
    p.wobble_phase = random.range(0.0f, 2.0f * std::numbers::pi_v<float>);
    p.shape = effect.shape;
    p.blend = effect.blend;
    return p;
}

std::uint32_t emit_burst(const ParticleEffect& effect, Vec2 position, float direction_degrees,
                         ParticleRandom& random, ParticlePool& pool) {
    std::uint32_t added = 0;
    for (std::uint32_t i = 0; i < effect.burst; ++i) {
        if (pool.spawn(make_particle(effect, position, direction_degrees, random))) {
            ++added;
        }
    }
    return added;
}

float direction_to_degrees(Vec2 direction) {
    return std::atan2(direction.y, direction.x) / kDegToRad;
}

}  // namespace wind::engine
