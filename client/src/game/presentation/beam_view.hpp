#pragma once

// 이 파일은 무엇을 하나요?
// 혜성 광선 연출 담당이에요. (명세 8.6, 6.3, 15d) 보여 주기 전용이라 게임 결과를 바꾸지 않아요.
// - 굵은 광선: 둥근 빛 그림을 4픽셀마다 겹쳐 놓아 부드러운 띠를 만들어요(바깥 얼음빛 · 가운데 혜성빛 · 하얀 심).
//   크기가 손끝에서 끝 쪽으로 흐르듯 출렁여서 힘이 뿜어 나가는 것처럼 보여요.
// - 켜질 때 0.08초 동안 굵어지고, 꺼질 때 0.12초 동안 가늘어지며 사라져요.
// - 손끝에 큰 빛, 벽에 막히면 끝에 큰 빛과 반짝 불꽃(0.07초마다), 띠 위로 반짝이가 흩날려요.
// - 조명: 손끝·가운데·끝 세 군데에 강한 빛 (밤·동굴에서 주변이 환해져요)
// - F4: 맞는 띠를 보라색으로 보여 줘요
// 모양은 FieldCombatFx가 HeroBeam(시뮬레이션)과 그리는 발 위치로 만들어 넘겨요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_emitter.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"

#include "wind/combat/spell.hpp"

#include <optional>
#include <vector>

namespace wind::game {

// 이번 화면의 광선 모양 (보여 주기용)
struct BeamShape {
    bool active = false;          // 쏘는 중인가
    engine::Vec2 hand;            // 손끝 (맵 위, 몸 높이, 그리는 발 기준)
    engine::Vec2 direction{1.0f, 0.0f};  // 쏘는 쪽 (길이 1)
    float length = 0.0f;          // 벽에서 잘린 길이 (픽셀)
    float width = 0.0f;           // 굵기 (픽셀)
    bool blocked = false;         // 벽에 막혔나 (끝에서 불꽃)
    engine::RectF hit_rect{};     // 맞는 띠 (F4)
    std::optional<combat::SpellLight> light;  // 광선 빛
};

class BeamView {
public:
    // sparkle: 띠 위 반짝이, impact: 벽에 닿은 끝의 불꽃 (없으면 그 효과만 안 보여요). 이 물건보다 오래 살아 있어야 해요.
    BeamView(const engine::ParticleEffect* sparkle, const engine::ParticleEffect* impact, engine::TextureHandle glow);

    // 화면 한 장마다: 켜짐·꺼짐 세기를 바꾸고 반짝이·불꽃을 뿜어요.
    // 이번에 막 켜졌으면 true (화면이 한 번 쿵 흔들려요)
    bool update(float dt_seconds, const BeamShape& shape, engine::ParticleRandom& random,
                engine::ParticlePool& air_fx);
    // 광선 그리기 (Glow 단계). show_boxes면 F4 띠
    void queue(const engine::Camera2D& camera, bool show_boxes, engine::RenderQueue& queue) const;
    // 조명에 더할 빛 (켜져 있거나 사라지는 중이면 세 개, 아니면 없음)
    [[nodiscard]] std::vector<engine::Light> lights() const;
    // 지금 보이는 세기 0~1 (켜질 때 0→1, 꺼질 때 1→0)
    [[nodiscard]] float strength() const { return strength_; }

private:
    // 빛 그림 하나를 size 크기로 center에 더하기 섞기로 놓아요
    void queue_glow(const engine::Camera2D& camera, engine::Vec2 center, float size, engine::Color tint,
                    engine::RenderQueue& queue) const;

    std::optional<engine::ParticleEmitter> sparkle_;
    const engine::ParticleEffect* impact_;
    engine::TextureHandle glow_;
    BeamShape shape_;           // 마지막으로 본 모양 (꺼진 뒤에도 사라지는 동안 그려요)
    float strength_ = 0.0f;     // 보이는 세기 0~1
    float clock_ = 0.0f;        // 출렁임·깜빡임 시계
    float impact_left_ = 0.0f;  // 다음 벽 불꽃까지 남은 시간
};

}  // namespace wind::game
