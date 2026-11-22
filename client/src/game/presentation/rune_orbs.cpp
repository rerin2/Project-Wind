// 이 파일은 무엇을 하나요?
// 룬 알갱이: 튀어나오기 -> 주인공에게 빨려 들기 -> 닿으면 사라지기.
// 수치의 이유는 docs/VISUALS.md "Runes" 절에 있어요.

#include "game/presentation/rune_orbs.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

constexpr std::int64_t kRunesPerOrb = 4;    // 알갱이 하나 = 룬 4개쯤 (12룬 슬라임 -> 3개)
constexpr int kMinOrbs = 3;
constexpr int kMaxOrbs = 10;
constexpr std::size_t kMaxAlive = 64;       // 한 화면 상한 (몬스터를 한꺼번에 많이 잡아도)
constexpr float kBurstSpeedMin = 40.0f;     // 튀어나오는 빠르기 (픽셀/초)
constexpr float kBurstSpeedMax = 90.0f;
constexpr float kLift = 8.0f;               // 몸 가운데쯤에서 나와요 (발에서 위로)
constexpr float kDriftSeconds = 0.3f;       // 이만큼 흩어진 뒤 빨려 들기 시작
constexpr float kDrag = 5.0f;               // 흩어지는 동안 느려져요
constexpr float kPull = 900.0f;             // 빨려 드는 힘 (픽셀/초²)
constexpr float kMaxSpeed = 420.0f;         // 가장 빠를 때
constexpr float kCatchRadius = 8.0f;        // 주인공 몸 가운데에서 이만큼이면 닿은 거예요
constexpr float kLifeSeconds = 2.0f;        // 못 닿아도 이만큼 지나면 사라져요
constexpr float kSize = 10.0f;              // 그림 크기 (픽셀)
constexpr engine::Color kOrbColor{255, 214, 120, 255};  // 금빛

}  // namespace

void RuneOrbs::spawn(engine::Vec2 at, std::int64_t amount, engine::ParticleRandom& random) {
    if (amount <= 0) return;
    const int count = static_cast<int>(std::clamp<std::int64_t>(amount / kRunesPerOrb, kMinOrbs, kMaxOrbs));
    for (int i = 0; i < count && orbs_.size() < kMaxAlive; ++i) {
        const float angle = random.range(0.0f, 2.0f * std::numbers::pi_v<float>);  // 사방으로
        const float speed = random.range(kBurstSpeedMin, kBurstSpeedMax);
        orbs_.push_back({{at.x, at.y - kLift}, {std::cos(angle) * speed, std::sin(angle) * speed}, 0.0f});
    }
}

void RuneOrbs::update(float dt_seconds, engine::Vec2 target) {
    for (auto& orb : orbs_) {
        orb.age += dt_seconds;
        if (orb.age < kDriftSeconds) {
            orb.velocity = orb.velocity * std::max(0.0f, 1.0f - kDrag * dt_seconds);  // 흩어지며 느려짐
        } else {
            const engine::Vec2 to_target = engine::normalized(target - orb.position);  // 주인공 쪽
            orb.velocity = orb.velocity + to_target * (kPull * dt_seconds);
            const float speed = engine::length(orb.velocity);
            if (speed > kMaxSpeed) orb.velocity = orb.velocity * (kMaxSpeed / speed);
        }
        orb.position = orb.position + orb.velocity * dt_seconds;
    }
    std::erase_if(orbs_, [target](const Orb& orb) {
        const bool caught = orb.age >= kDriftSeconds && engine::length(target - orb.position) <= kCatchRadius;
        return caught || orb.age >= kLifeSeconds;
    });
}

void RuneOrbs::queue(const engine::Camera2D& camera, engine::TextureHandle glow, engine::RenderQueue& queue) const {
    for (const auto& orb : orbs_) {
        const engine::Vec2 screen = camera.world_to_screen(orb.position);
        engine::SpriteDraw sprite;
        sprite.texture = glow;
        sprite.dest = {std::round(screen.x - kSize * 0.5f), std::round(screen.y - kSize * 0.5f), kSize, kSize};
        sprite.tint = kOrbColor;
        sprite.blend = engine::BlendMode::Add;  // 겹치면 더 밝게 반짝여요
        queue.push_sprite(engine::DrawLayer::Glow, orb.position.y, sprite);  // 조명 뒤라 밤에도 잘 보여요
    }
}

}  // namespace wind::game
