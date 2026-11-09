// 이 파일은 무엇을 하나요?
// 먼지를 언제, 어느 방향으로 터뜨릴지 정하는 곳이에요.
// 먼지 하나하나의 모양·색·수명은 효과 파일(content/fx/dust_*.json)에 있어요.

#include "game/presentation/dust_emitter.hpp"

#include "engine/fx/particle_spawn.hpp"

namespace wind::game {
namespace {

constexpr float kPuffEveryPixels = 14.0f;  // 이만큼 걸을 때마다 먼지 한 번
constexpr float kMinWalkSpeed = 40.0f;     // 이보다 느리면 먼지가 안 나요 (살금살금)

}  // namespace

DustEmitter::DustEmitter(const engine::ParticleEffect* walk, const engine::ParticleEffect* dodge)
    : walk_(walk), dodge_(dodge) {}

void DustEmitter::walk(engine::Vec2 feet, engine::Vec2 velocity, float dt_seconds,
                       engine::ParticleRandom& random, engine::ParticlePool& pool) {
    const float speed = engine::length(velocity);
    if (walk_ == nullptr || speed < kMinWalkSpeed) {
        distance_since_puff_ = 0.0f;
        return;
    }
    distance_since_puff_ += speed * dt_seconds;
    if (distance_since_puff_ < kPuffEveryPixels) {
        return;
    }
    distance_since_puff_ = 0.0f;
    // 먼지는 걷는 반대쪽으로 살짝 밀려나요 (발로 땅을 차는 느낌).
    const float backward = engine::direction_to_degrees(velocity * -1.0f);
    engine::emit_burst(*walk_, feet, backward, random, pool);
}

void DustEmitter::dodge_burst(engine::Vec2 feet, engine::Vec2 direction,
                              engine::ParticleRandom& random, engine::ParticlePool& pool) {
    if (dodge_ == nullptr) {
        return;
    }
    const float backward = engine::direction_to_degrees(direction * -1.0f);
    engine::emit_burst(*dodge_, feet, backward, random, pool);
}

}  // namespace wind::game
