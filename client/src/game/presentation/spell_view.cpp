// 이 파일은 무엇을 하나요?
// 시전 손끝 빛, 날아가는 빛 탄, 꼬리·반짝, 마술 빛을 만들고 그리는 곳이에요.
// 색과 크기 값의 이유는 docs/VISUALS.md "Star shard" 절에 있어요.

#include "game/presentation/spell_view.hpp"

#include "engine/fx/particle_spawn.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kHaloSize = 20.0f;                       // 빛무리 지름 (픽셀)
constexpr float kCoreSize = 7.0f;                        // 하얀 심 지름
constexpr float kTwinkleHz = 9.0f;                       // 빛무리가 1초에 몇 번 반짝이는지
constexpr float kTwinkleDepth = 0.12f;                   // 반짝일 때 크기가 얼마나 변하는지 (12%)
constexpr engine::Color kHaloTint{168, 220, 239, 210};   // 팔레트 ice #a8dcef
constexpr engine::Color kCoreTint{242, 246, 247, 255};   // 팔레트 snow #f2f6f7
constexpr float kCastMinSize = 4.0f;                     // 시전 손끝 빛: 처음 지름
constexpr float kCastMaxSize = 14.0f;                    // 다 모았을 때 지름
constexpr float kCastLightScale = 0.5f;                  // 손끝 빛은 빛 탄 빛의 절반 크기
constexpr float kTrailArea = 2.0f;                       // 꼬리가 나오는 네모 크기 (탄 가운데)
constexpr engine::Color kBoxColor{170, 120, 255, 140};   // F4: 빛 탄 네모 (반투명 보라)

engine::Color to_color(world::Rgba c) {
    return {c.r, c.g, c.b, c.a};
}

}  // namespace

engine::Vec2 drawn_center(const Projectile& shot, float alpha) {
    const float t = std::clamp(alpha, 0.0f, 1.0f);
    const engine::Vec2 ground = shot.previous_ground() + (shot.ground() - shot.previous_ground()) * t;
    return {ground.x, ground.y - shot.shot().lift};  // 몸 높이로 올려요
}

SpellView::SpellView(const engine::ParticleEffect* trail, const engine::ParticleEffect* burst,
                     engine::TextureHandle glow)
    : burst_(burst), glow_(glow) {
    if (trail != nullptr) {
        trail_.emplace(*trail);
    }
}

void SpellView::update(float dt_seconds, std::span<const Projectile> shots, float alpha,
                       std::span<const ProjectileEndEvent> ends, std::optional<CastGlow> cast,
                       engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    clock_ += dt_seconds;
    cast_ = cast;
    if (trail_) {
        for (const auto& shot : shots) {  // 빛 탄마다 가운데 작은 네모에서 꼬리를 뿜어요
            const engine::Vec2 c = drawn_center(shot, alpha);
            trail_->update(dt_seconds, {c.x - kTrailArea * 0.5f, c.y - kTrailArea * 0.5f, kTrailArea, kTrailArea},
                           random, air_fx);
        }
    }
    if (burst_ != nullptr) {
        for (const auto& end : ends) {  // 사라진 자리에서 반짝 (사방으로)
            (void)engine::emit_burst(*burst_, end.at, 0.0f, random, air_fx);
        }
    }
}

void SpellView::queue(const engine::Camera2D& camera, std::span<const Projectile> shots, float alpha,
                      bool show_boxes, engine::RenderQueue& queue) const {
    if (cast_) {  // 손끝 빛: 모을수록 커져요
        const float size = kCastMinSize + (kCastMaxSize - kCastMinSize) * cast_->progress;
        queue_glow(camera, cast_->at, size * 1.8f, kHaloTint, queue);
        queue_glow(camera, cast_->at, size * 0.6f, kCoreTint, queue);
    }
    // 빛무리 크기가 살짝 커졌다 작아졌다 해요 (별이 반짝이듯)
    const float twinkle = 1.0f + kTwinkleDepth * std::sin(clock_ * kTwinkleHz * 6.2831853f);
    for (const auto& shot : shots) {
        const engine::Vec2 c = drawn_center(shot, alpha);
        queue_glow(camera, c, kHaloSize * twinkle, kHaloTint, queue);
        queue_glow(camera, c, kCoreSize, kCoreTint, queue);
        if (show_boxes) {  // F4: 맞는 네모 (지금 박자 위치)
            const engine::RectF box = shot.rect();
            const engine::Vec2 top_left = camera.world_to_screen({box.x, box.y});
            queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, {top_left.x, top_left.y, box.w, box.h},
                            kBoxColor);
        }
    }
}

std::vector<engine::Light> SpellView::lights(std::span<const Projectile> shots, float alpha) const {
    std::vector<engine::Light> result;
    // 빛은 마술마다 달라서 빛 탄·손끝 빛이 저마다 들고 다녀요 (15b: Q로 마술을 바꿔도 날아가는 탄은 제 빛 그대로)
    auto make = [](const combat::SpellLight& light, engine::Vec2 at, float radius_scale, float intensity_scale) {
        engine::Light out;
        out.position = at;
        out.radius = light.radius * radius_scale;
        out.color = to_color(light.color);
        out.intensity = light.intensity * intensity_scale;
        return out;
    };
    for (const auto& shot : shots) {
        if (shot.shot().light) result.push_back(make(*shot.shot().light, drawn_center(shot, alpha), 1.0f, 1.0f));
    }
    if (cast_ && cast_->light) {  // 손끝 빛: 모을수록 밝아져요
        result.push_back(make(*cast_->light, cast_->at, kCastLightScale, cast_->progress));
    }
    return result;
}

void SpellView::queue_glow(const engine::Camera2D& camera, engine::Vec2 center, float size,
                           engine::Color tint, engine::RenderQueue& queue) const {
    if (!glow_.valid()) {
        return;  // 빛 그림이 없으면 안 그려요
    }
    const engine::Vec2 screen = camera.world_to_screen(center);
    engine::SpriteDraw draw;
    draw.texture = glow_;
    draw.dest = {std::round(screen.x - size * 0.5f), std::round(screen.y - size * 0.5f), size, size};
    draw.tint = tint;
    draw.blend = engine::BlendMode::Add;                     // 더하기 섞기: 빛나 보여요
    queue.push_sprite(engine::DrawLayer::Glow, 0.0f, draw);  // 조명 뒤라 밤·동굴에서도 밝아요
}

}  // namespace wind::game
