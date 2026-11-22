// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 연출: 쿠션 그림, 피어나기, 쉬는 동안의 초록 빛·하트·z.
// 수치의 이유는 docs/VISUALS.md "Cat-nip cushion" 절에 있어요.

#include "game/presentation/cushion_view.hpp"

#include "engine/fx/particle_spawn.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <utility>

namespace wind::game {
namespace {

// 그림 판: 칸 32x32, 발 자리(앉는 곳)는 칸의 (16, 28). 0 시든, 1 피어난, 2 z, 3 하트
constexpr int kFrame = 32;
constexpr float kOriginX = 16.0f;
constexpr float kOriginY = 28.0f;
constexpr int kWitheredFrame = 0;
constexpr int kBloomedFrame = 1;
constexpr int kZFrame = 2;
constexpr int kHeartFrame = 3;
// 쿠션은 주인공보다 아주 조금 먼저(아래에) 그려요. 같은 발 높이에 앉아도 주인공이 위에 보이게.
constexpr float kBehindHero = 0.5f;
// 피어나기: 0.45초 동안 1 -> 1.3 -> 1배로 퐁 (사인 반 바퀴), 빛은 0.8초 동안 번쩍였다 사그라들어요
constexpr float kBloomSeconds = 0.45f;
constexpr float kBloomScale = 0.3f;
constexpr float kBloomLightSeconds = 0.8f;
// 쉬는 동안: 앉는 순간 하트 2개, 웅크려 졸면(1.4초 뒤) 0.8초마다 z 하나
constexpr float kDozeAfterSeconds = 1.4f;   // PlayerSprite가 웅크리는 때와 같아요
constexpr float kZEverySeconds = 0.8f;
constexpr float kZLife = 1.6f;              // z가 떠 있는 시간
constexpr float kZRise = 16.0f;             // z가 떠오르는 높이 (픽셀)
constexpr float kZSway = 3.0f;              // z가 옆으로 흔들리는 폭 (픽셀)
constexpr float kHeartLife = 0.9f;
constexpr float kHeartRise = 12.0f;
constexpr float kHeadHeight = 34.0f;        // 앉은 머리 높이 (발에서 위로, 픽셀)
constexpr float kFadeFrom = 0.6f;           // 수명의 60%부터 흐려져요
constexpr std::size_t kMaxFloaters = 12;    // 떠 있는 z·하트 상한
// 초록 빛 (쉬는 동안): 민트색, 서서히 켜지고 숨 쉬듯 (0.5 Hz)
constexpr engine::Color kRestLightColor{120, 232, 160, 255};
constexpr float kRestLightRadius = 60.0f;
constexpr float kRestLightIntensity = 0.85f;
constexpr float kRestLightFadeIn = 0.6f;    // 이 시간에 걸쳐 켜져요
constexpr float kRestLightPulseHz = 0.5f;
constexpr float kRestLightPulseDepth = 0.25f;
constexpr float kCushionLift = 10.0f;       // 빛 가운데: 발보다 조금 위 (쿠션 윗면)
// 피어나는 순간의 빛: 연보라
constexpr engine::Color kBloomLightColor{214, 170, 255, 255};
constexpr float kBloomLightRadius = 72.0f;

// 0~1 수명 비율에서 투명도: 처음엔 또렷, 끝 40%에서 흐려져요
float fade(float t) {
    return t < kFadeFrom ? 1.0f : std::clamp((1.0f - t) / (1.0f - kFadeFrom), 0.0f, 1.0f);
}

}  // namespace

CushionView::CushionView(std::vector<Cushion> cushions, const std::function<bool(std::string_view)>& discovered,
                         engine::TextureHandle texture, const engine::ParticleEffect* bloom_fx)
    : cushions_(std::move(cushions)), looks_(cushions_.size()), texture_(texture), bloom_fx_(bloom_fx) {
    for (std::size_t i = 0; i < looks_.size(); ++i) looks_[i].bloomed = discovered(cushions_[i].id);  // 찾은 쿠션은 피어난 모습
}

void CushionView::bloom(std::string_view cushion_id) {
    for (std::size_t i = 0; i < cushions_.size(); ++i) {
        if (cushions_[i].id != cushion_id || looks_[i].bloomed) continue;
        looks_[i].bloomed = true;      // 바로 피어난 그림으로 바꾸고
        looks_[i].bloom_age = 0.0f;    // 퐁 연출 시작
        burst_queue_.push_back(i);     // 꽃잎은 다음 update에서 (주사위가 필요해요)
    }
}

void CushionView::spawn(FloaterKind kind, engine::Vec2 seat, float drift) {
    if (floaters_.size() >= kMaxFloaters) return;  // 너무 많으면 새로 만들지 않아요
    floaters_.push_back({kind, {seat.x, seat.y - kHeadHeight}, 0.0f, drift});
}

void CushionView::update(float dt_seconds, const Cushion* resting, const HeroRest& rest,
                         engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    // 1) 피어나기: 퐁 시계, 꽃잎 터뜨리기
    for (auto& look : looks_) {
        if (look.bloom_age >= 0.0f) look.bloom_age += dt_seconds;
        if (look.bloom_age > kBloomLightSeconds) look.bloom_age = -1.0f;  // 연출 끝
    }
    for (const std::size_t i : burst_queue_) {
        if (bloom_fx_ != nullptr) {
            (void)engine::emit_burst(*bloom_fx_, {cushions_[i].seat.x, cushions_[i].seat.y - kCushionLift}, 0.0f,
                                     random, air_fx);
        }
    }
    burst_queue_.clear();
    // 2) 쉬기: 앉는 순간 하트, 졸면 z
    resting_ = resting;
    const HeroRest::Phase phase = resting != nullptr ? rest.phase() : HeroRest::Phase::None;
    if (resting != nullptr && last_phase_ == HeroRest::Phase::None && phase != HeroRest::Phase::None) {
        spawn(FloaterKind::Heart, resting->seat, -0.6f);  // 하트 두 개가 양쪽으로
        spawn(FloaterKind::Heart, resting->seat, 0.6f);
        z_timer_ = kZEverySeconds * 0.5f;                 // 첫 z는 졸기 시작하고 조금 뒤
    }
    const bool dozing = phase == HeroRest::Phase::Sitting && rest.phase_seconds() >= kDozeAfterSeconds;
    if (dozing) {
        z_timer_ -= dt_seconds;
        if (z_timer_ <= 0.0f) {
            z_timer_ += kZEverySeconds;
            spawn(FloaterKind::Z, resting->seat, (floaters_.size() % 2 == 0) ? 1.0f : -1.0f);
        }
    }
    last_phase_ = phase;
    glow_clock_ = resting != nullptr ? glow_clock_ + dt_seconds : 0.0f;
    // 3) 떠 있는 것들 나이 먹이기, 다 산 것은 빼기
    for (auto& floater : floaters_) floater.age += dt_seconds;
    std::erase_if(floaters_, [](const Floater& f) {
        return f.age >= (f.kind == FloaterKind::Z ? kZLife : kHeartLife);
    });
}

void CushionView::queue_floater(const engine::Camera2D& camera, engine::RenderQueue& queue,
                                const Floater& floater) const {
    const bool z = floater.kind == FloaterKind::Z;
    const float t = floater.age / (z ? kZLife : kHeartLife);  // 0~1 수명 비율
    const float rise = (z ? kZRise : kHeartRise) * t;
    const float sway = z ? std::sin(t * 2.0f * std::numbers::pi_v<float>) * kZSway * floater.drift
                         : floater.drift * 6.0f * t;  // 하트는 양옆으로 벌어지며 떠올라요
    const engine::Vec2 world{floater.start.x + sway, floater.start.y - rise};
    const engine::Vec2 screen = camera.world_to_screen(world);
    engine::SpriteDraw sprite;
    sprite.texture = texture_;
    sprite.source = {(z ? kZFrame : kHeartFrame) * kFrame, 0, kFrame, kFrame};
    sprite.dest = {std::round(screen.x - kFrame * 0.5f), std::round(screen.y - kFrame * 0.5f),
                   static_cast<float>(kFrame), static_cast<float>(kFrame)};
    sprite.tint.a = static_cast<std::uint8_t>(std::lround(fade(t) * 255.0f));
    queue.push_sprite(engine::DrawLayer::WorldFx, world.y, sprite);  // 나뭇잎보다 위
}

void CushionView::queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const {
    for (std::size_t i = 0; i < cushions_.size(); ++i) {
        const Look& look = looks_[i];
        // 퐁: 사인 반 바퀴로 1 -> 1.3 -> 1배 (발 자리를 중심으로 커져요)
        float scale = 1.0f;
        if (look.bloom_age >= 0.0f && look.bloom_age < kBloomSeconds) {
            scale += kBloomScale * std::sin(look.bloom_age / kBloomSeconds * std::numbers::pi_v<float>);
        }
        const engine::Vec2 screen = camera.world_to_screen(cushions_[i].seat);
        engine::SpriteDraw sprite;
        sprite.texture = texture_;
        sprite.source = {(look.bloomed ? kBloomedFrame : kWitheredFrame) * kFrame, 0, kFrame, kFrame};
        sprite.dest = {std::round(screen.x - kOriginX * scale), std::round(screen.y - kOriginY * scale),
                       std::round(kFrame * scale), std::round(kFrame * scale)};
        queue.push_sprite(engine::DrawLayer::YSorted, cushions_[i].seat.y - kBehindHero, sprite);
    }
    for (const auto& floater : floaters_) queue_floater(camera, queue, floater);
}

std::vector<engine::Light> CushionView::lights() const {
    std::vector<engine::Light> lights;
    if (resting_ != nullptr) {  // 쉬는 쿠션: 민트색 빛이 서서히 켜지고 숨 쉬어요
        engine::Light light;
        light.position = {resting_->seat.x, resting_->seat.y - kCushionLift};
        light.radius = kRestLightRadius;
        light.color = kRestLightColor;
        light.intensity = kRestLightIntensity * std::min(1.0f, glow_clock_ / kRestLightFadeIn);
        light.pulse_frequency = kRestLightPulseHz;
        light.pulse_depth = kRestLightPulseDepth;
        lights.push_back(light);
    }
    for (std::size_t i = 0; i < cushions_.size(); ++i) {  // 피어나는 순간: 연보라 빛이 번쩍했다 사그라들어요
        if (looks_[i].bloom_age < 0.0f) continue;
        engine::Light light;
        light.position = {cushions_[i].seat.x, cushions_[i].seat.y - kCushionLift};
        light.radius = kBloomLightRadius;
        light.color = kBloomLightColor;
        light.intensity = 1.0f - looks_[i].bloom_age / kBloomLightSeconds;
        lights.push_back(light);
    }
    return lights;
}

}  // namespace wind::game
