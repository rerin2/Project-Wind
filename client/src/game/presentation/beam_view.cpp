// 이 파일은 무엇을 하나요?
// 혜성 광선의 빛 띠, 손끝·끝 빛, 반짝이, 벽 불꽃, 조명을 만들고 그리는 곳이에요.
// 색과 크기 값의 이유는 docs/VISUALS.md "Comet beam" 절에 있어요.

#include "game/presentation/beam_view.hpp"

#include "engine/fx/particle_spawn.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kOpenSeconds = 0.08f;    // 켜질 때 굵어지는 시간
constexpr float kCloseSeconds = 0.12f;   // 꺼질 때 가늘어지는 시간
constexpr float kBeadSpacing = 4.0f;     // 빛 구슬 사이 (픽셀): 작을수록 매끈하지만 그림이 많아져요
constexpr float kOuterScale = 2.0f;      // 바깥 얼음빛 지름 = 굵기 × 이 값
constexpr float kMidScale = 1.15f;       // 가운데 혜성빛
constexpr float kCoreScale = 0.55f;      // 하얀 심
constexpr float kWaveDepth = 0.18f;      // 출렁임 세기 (지름 ±18%)
constexpr float kWaveNumber = 0.09f;     // 출렁임 물결 촘촘함 (1픽셀에 라디안)
constexpr float kWaveSpeed = 22.0f;      // 출렁임이 앞으로 흐르는 빠르기 (라디안/초)
constexpr float kTipTaper = 18.0f;       // 막히지 않은 끝에서 이만큼 가늘어져요 (픽셀)
constexpr float kHandFlare = 2.6f;       // 손끝 빛 지름 = 굵기 × 이 값
constexpr float kImpactFlare = 3.0f;     // 벽 끝 빛 지름
constexpr float kImpactInterval = 0.07f; // 벽 불꽃 간격 (초)
constexpr float kSparkleRate = 3.0f;     // 반짝이는 별빛 꼬리 효과의 3배로 뿜어요 (띠가 길어서)
constexpr float kFlickerHz = 31.0f;      // 빛 떨림 빠르기
constexpr float kFlickerDepth = 0.08f;   // 빛 떨림 세기
constexpr float kMidLightScale = 1.2f;   // 가운데 빛은 조금 더 넓게
constexpr engine::Color kOuterTint{168, 220, 239, 130};  // 팔레트 ice #a8dcef, 반투명
constexpr engine::Color kCoreTint{242, 246, 247, 255};   // 팔레트 snow #f2f6f7
constexpr engine::Color kBoxColor{170, 120, 255, 140};   // F4: 맞는 띠 (빛 탄과 같은 보라)
constexpr engine::Color kDefaultMid{200, 240, 255, 200}; // 빛 데이터가 없을 때 가운데 색 (#c8f0ff)

// 0~1을 부드럽게 (처음과 끝이 느림)
float smooth(float t) {
    const float x = std::clamp(t, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

}  // namespace

BeamView::BeamView(const engine::ParticleEffect* sparkle, const engine::ParticleEffect* impact,
                   engine::TextureHandle glow)
    : impact_(impact), glow_(glow) {
    if (sparkle != nullptr) sparkle_.emplace(*sparkle);
}

bool BeamView::update(float dt_seconds, const BeamShape& shape, engine::ParticleRandom& random,
                      engine::ParticlePool& air_fx) {
    const float dt = std::max(0.0f, dt_seconds);
    clock_ += dt;
    const bool opened = shape.active && !shape_.active;  // 이번에 막 켜졌나
    if (shape.active) {
        shape_ = shape;  // 켜져 있으면 새 모양, 꺼졌으면 마지막 모양 그대로 사라져요
        strength_ = std::min(1.0f, strength_ + dt / kOpenSeconds);
    } else {
        shape_.active = false;
        strength_ = std::max(0.0f, strength_ - dt / kCloseSeconds);
    }
    if (!shape.active || shape_.length <= 0.0f) return opened;
    const engine::Vec2 tip = shape_.hand + shape_.direction * shape_.length;
    if (sparkle_) {  // 띠 위로 반짝이 (띠를 감싼 네모 안 아무 데서나)
        const engine::RectF area{std::min(shape_.hand.x, tip.x) - shape_.width * 0.5f,
                                 std::min(shape_.hand.y, tip.y) - shape_.width * 0.5f,
                                 std::abs(tip.x - shape_.hand.x) + shape_.width,
                                 std::abs(tip.y - shape_.hand.y) + shape_.width};
        (void)sparkle_->update(dt, area, random, air_fx, kSparkleRate);
    }
    impact_left_ -= dt;
    if (shape_.blocked && impact_ != nullptr && impact_left_ <= 0.0f) {  // 벽에 부딪힌 끝에서 불꽃
        (void)engine::emit_burst(*impact_, tip, 0.0f, random, air_fx);
        impact_left_ = kImpactInterval;
    }
    return opened;
}

void BeamView::queue(const engine::Camera2D& camera, bool show_boxes, engine::RenderQueue& queue) const {
    if (strength_ <= 0.0f || shape_.width <= 0.0f) return;
    const float width = shape_.width * smooth(strength_);  // 켜질 때 굵어지고 꺼질 때 가늘어져요
    const engine::Color mid = shape_.light
                                  ? engine::Color{shape_.light->color.r, shape_.light->color.g, shape_.light->color.b, 200}
                                  : kDefaultMid;
    // 1) 띠: 손끝에서 끝까지 빛 구슬을 겹쳐요. 지름이 앞으로 흐르듯 출렁여요
    for (float d = 0.0f; d <= shape_.length; d += kBeadSpacing) {
        const float wave = 1.0f + kWaveDepth * std::sin(d * kWaveNumber - clock_ * kWaveSpeed);
        // 막히지 않은 끝은 가늘어지며 사라지고, 벽에 막힌 끝은 굵은 그대로 부딪혀요
        const float taper = shape_.blocked ? 1.0f : std::clamp((shape_.length - d) / kTipTaper, 0.25f, 1.0f);
        const float size = width * wave * taper;
        const engine::Vec2 at = shape_.hand + shape_.direction * d;
        queue_glow(camera, at, size * kOuterScale, kOuterTint, queue);
        queue_glow(camera, at, size * kMidScale, mid, queue);
        queue_glow(camera, at, size * kCoreScale, kCoreTint, queue);
    }
    // 2) 손끝 빛 (빠르게 떨려요)
    const float flicker = 1.0f + kFlickerDepth * std::sin(clock_ * kFlickerHz);
    queue_glow(camera, shape_.hand, width * kHandFlare * flicker, kOuterTint, queue);
    queue_glow(camera, shape_.hand, width * kHandFlare * 0.5f * flicker, kCoreTint, queue);
    // 3) 벽에 부딪힌 끝 빛
    if (shape_.blocked) {
        const engine::Vec2 tip = shape_.hand + shape_.direction * shape_.length;
        queue_glow(camera, tip, width * kImpactFlare / flicker, mid, queue);
        queue_glow(camera, tip, width * kImpactFlare * 0.45f, kCoreTint, queue);
    }
    if (show_boxes && shape_.active) {  // F4: 맞는 띠 (시뮬레이션 모양)
        const engine::Vec2 top_left = camera.world_to_screen({shape_.hit_rect.x, shape_.hit_rect.y});
        queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f,
                        {top_left.x, top_left.y, shape_.hit_rect.w, shape_.hit_rect.h}, kBoxColor);
    }
}

std::vector<engine::Light> BeamView::lights() const {
    if (strength_ <= 0.0f || !shape_.light) return {};
    const auto& data = *shape_.light;
    const float flicker = 1.0f - kFlickerDepth + kFlickerDepth * std::sin(clock_ * kFlickerHz);
    auto make = [&](engine::Vec2 at, float radius_scale) {
        engine::Light light;
        light.position = at;
        light.radius = data.radius * radius_scale;
        light.color = {data.color.r, data.color.g, data.color.b, 255};
        light.intensity = data.intensity * strength_ * flicker;
        return light;
    };
    const engine::Vec2 tip = shape_.hand + shape_.direction * shape_.length;
    return {make(shape_.hand, 1.0f), make(shape_.hand + shape_.direction * (shape_.length * 0.5f), kMidLightScale),
            make(tip, 1.0f)};  // 손끝·가운데·끝: 띠 전체가 주변을 밝혀요
}

void BeamView::queue_glow(const engine::Camera2D& camera, engine::Vec2 center, float size, engine::Color tint,
                          engine::RenderQueue& queue) const {
    if (!glow_.valid() || size <= 0.0f) return;  // 빛 그림이 없으면 안 그려요
    const engine::Vec2 screen = camera.world_to_screen(center);
    engine::SpriteDraw draw;
    draw.texture = glow_;
    draw.dest = {std::round(screen.x - size * 0.5f), std::round(screen.y - size * 0.5f), size, size};
    draw.tint = tint;
    draw.blend = engine::BlendMode::Add;                     // 더하기 섞기: 빛나 보여요
    queue.push_sprite(engine::DrawLayer::Glow, 0.0f, draw);  // 조명 뒤라 밤·동굴에서도 밝아요
}

}  // namespace wind::game
