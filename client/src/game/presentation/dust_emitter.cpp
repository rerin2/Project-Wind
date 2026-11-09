// 이 파일은 무엇을 하나요?
// 먼지 점(파티클)을 언제, 어디서, 어떤 모양으로 만들지 정하는 곳이에요.
// 숫자는 docs/VISUALS.md 의 "먼지" 표와 같아요. 바꾸면 그 표도 같이 고쳐요.

#include "game/presentation/dust_emitter.hpp"

#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

constexpr float kPuffEveryPixels = 14.0f;  // 이만큼 걸을 때마다 먼지 한 줌
constexpr float kMinWalkSpeed = 40.0f;     // 이보다 느리면 먼지가 안 나요 (살금살금)
constexpr int kBurstCount = 10;            // 구를 때 한꺼번에 나오는 먼지 수

}  // namespace

DustEmitter::DustEmitter(std::uint32_t seed) : rng_(seed) {}

float DustEmitter::random(float a, float b) {
    std::uniform_real_distribution<float> dist(a, b);
    return dist(rng_);
}

void DustEmitter::walk(engine::Vec2 feet, engine::Vec2 velocity, float dt_seconds,
                       engine::ParticlePool& pool) {
    const float speed = engine::length(velocity);
    if (speed < kMinWalkSpeed) {
        distance_since_puff_ = 0.0f;
        return;
    }
    distance_since_puff_ += speed * dt_seconds;
    if (distance_since_puff_ < kPuffEveryPixels) {
        return;
    }
    distance_since_puff_ = 0.0f;
    // 먼지는 걷는 반대쪽으로 살짝 밀려나요 (발로 땅을 차는 느낌).
    const engine::Vec2 back = engine::normalized(velocity) * -12.0f;
    puff(feet, back, pool);
}

void DustEmitter::dodge_burst(engine::Vec2 feet, engine::Vec2 direction,
                              engine::ParticlePool& pool) {
    const engine::Vec2 back = engine::normalized(direction) * -40.0f;
    for (int i = 0; i < kBurstCount; ++i) {
        // 뒤쪽을 중심으로 부채꼴처럼 퍼지게 해요.
        const float angle = random(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
        const engine::Vec2 spread{std::cos(angle) * 30.0f, std::sin(angle) * 14.0f};
        puff(feet, back + spread, pool);
    }
}

void DustEmitter::puff(engine::Vec2 feet, engine::Vec2 push, engine::ParticlePool& pool) {
    engine::Particle p;
    p.position = {feet.x + random(-3.0f, 3.0f), feet.y + random(-2.0f, 1.0f)};
    p.velocity = {push.x + random(-8.0f, 8.0f), push.y + random(-14.0f, -4.0f)};
    p.drag = 5.0f;            // 금방 느려지고
    p.gravity = -10.0f;       // 살짝 위로 떠올라요
    p.lifetime = random(0.35f, 0.55f);
    p.start_size = random(4.0f, 6.0f);
    p.end_size = 1.0f;
    p.start_alpha = 170;
    pool.spawn(p);
}

}  // namespace wind::game
