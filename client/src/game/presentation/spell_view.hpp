#pragma once

// 이 파일은 무엇을 하나요?
// 마술 연출 담당이에요. (명세 8.6, 6.3) 보여 주기 전용이라 게임 결과를 바꾸지 않아요.
// - 시전 중: 손끝에 작은 빛이 점점 커져요.
// - 날아가는 빛 탄: 하늘색 빛무리 + 하얀 심 (더하기 섞기라 반짝여 보여요), 뒤로 반짝이 꼬리
// - 사라질 때: 그 자리에서 작은 반짝 터짐
// - 빛 탄과 손끝 빛은 조명에 빛을 더해요 (밤·동굴에서 주변을 밝혀요)
// - F4: 빛 탄이 맞는 네모를 보라색으로 보여 줘요
// 그림 위치는 두 박자 사이를 이어서(alpha) 부드럽게 그려요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_emitter.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/combat/projectile.hpp"

#include "wind/combat/spell.hpp"

#include <optional>
#include <span>
#include <vector>

namespace wind::game {

// 시전 중인 손끝 빛
struct CastGlow {
    engine::Vec2 at;        // 손끝 (맵 위, 몸 높이)
    float progress = 0.0f;  // 모은 정도 0~1
    std::optional<combat::SpellLight> light;  // 시전 중인 마술의 빛 (없으면 조명에 안 더해요, 15b)
};

class SpellView {
public:
    // trail / burst: 꼬리·반짝 효과 (없으면 그 효과만 안 보여요). 이 물건보다 오래 살아 있어야 해요.
    // glow: 둥근 빛 그림. 마술이 내는 빛은 빛 탄·손끝 빛이 저마다 들고 와요 (15b, 없으면 조명에 안 더해요)
    SpellView(const engine::ParticleEffect* trail, const engine::ParticleEffect* burst,
              engine::TextureHandle glow);

    // 화면 한 장마다: 꼬리를 뿜고, 사라진 탄 자리에서 반짝을 터뜨리고, 손끝 빛을 기억해요.
    void update(float dt_seconds, std::span<const Projectile> shots, float alpha,
                std::span<const ProjectileEndEvent> ends, std::optional<CastGlow> cast,
                engine::ParticleRandom& random, engine::ParticlePool& air_fx);
    // 빛 탄·손끝 빛 그리기 (Glow 단계), show_boxes면 F4 네모
    void queue(const engine::Camera2D& camera, std::span<const Projectile> shots, float alpha,
               bool show_boxes, engine::RenderQueue& queue) const;
    // 조명에 더할 빛들 (빛 탄마다 하나, 시전 중이면 손끝에 작은 빛)
    [[nodiscard]] std::vector<engine::Light> lights(std::span<const Projectile> shots, float alpha) const;

private:
    // 빛 그림 하나를 size 크기로 center에 더하기 섞기로 놓아요
    void queue_glow(const engine::Camera2D& camera, engine::Vec2 center, float size,
                    engine::Color tint, engine::RenderQueue& queue) const;

    std::optional<engine::ParticleEmitter> trail_;  // 꼬리 방출기 (빛 탄마다 같이 써요)
    const engine::ParticleEffect* burst_;           // 사라질 때 반짝
    engine::TextureHandle glow_;
    std::optional<CastGlow> cast_;                  // 이번 화면의 손끝 빛
    float clock_ = 0.0f;                            // 반짝임 시계
};

// 그릴 위치: 지난 박자와 지금 박자 사이를 alpha만큼 이은 몸 높이 가운데
[[nodiscard]] engine::Vec2 drawn_center(const Projectile& shot, float alpha);

}  // namespace wind::game
