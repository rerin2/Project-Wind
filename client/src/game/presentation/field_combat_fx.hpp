#pragma once

// 이 파일은 무엇을 하나요?
// 필드 싸움 연출을 한데 모은 담당이에요. (9c, 9e, 10b, 11a) 보여 주기만 하고 게임 결과는 바꾸지 않아요.
// - 허수아비 그림·HP 바·번쩍임 (CombatView)
// - 몬스터 그림: 숨쉬기·통통 뛰기, 느낌표, 공격 예고 바닥 표시 (MonsterView, 11a·11b)
// - 칼자국 (SlashArc), 손끝 빛·빛 탄·꼬리 (SpellView)
// - 타격감 (10b): 화면 흔들림(ScreenShake), 피해 숫자(DamageNumbers), 타격 불꽃(hit_spark 파티클)
// 박자마다 생긴 "맞은 일"과 "사라진 빛 탄"을 모아 두었다가(collect), 화면 한 장마다 한꺼번에 연출해요.
// (한 화면에 박자가 0번이나 여러 번이어도 빠지는 일이 없어요)
// field_state.cpp가 300줄을 넘지 않게 싸움 연출을 여기로 옮겼어요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/presentation/combat_view.hpp"
#include "game/presentation/damage_numbers.hpp"
#include "game/presentation/feel_settings.hpp"
#include "game/presentation/monster_view.hpp"
#include "game/presentation/screen_shake.hpp"
#include "game/presentation/slash_arc.hpp"
#include "game/presentation/spell_view.hpp"
#include "game/states/field_combat.hpp"

#include "wind/combat/spell.hpp"

#include <optional>
#include <vector>

namespace wind::game {

// 싸움 연출 준비물 (그림과 효과). 효과 포인터는 이 담당보다 오래 살아 있어야 해요.
struct CombatFxSetup {
    engine::TextureHandle shadow;                       // 발밑 그림자
    engine::TextureHandle slash;                        // 칼자국 반달
    engine::TextureHandle glow;                         // 둥근 빛 (빛 탄)
    std::vector<engine::TextureHandle> dummies;         // 허수아비마다 그림
    std::vector<engine::TextureHandle> monsters;        // 몬스터마다 그림 (11a)
    const engine::ParticleEffect* star_trail = nullptr;  // 빛 탄 꼬리
    const engine::ParticleEffect* star_burst = nullptr;  // 빛 탄이 사라질 때
    const engine::ParticleEffect* hit_spark = nullptr;   // 맞을 때 불꽃
    std::optional<combat::SpellLight> spell_light;       // 마술이 내는 빛
};

class FieldCombatFx {
public:
    explicit FieldCombatFx(CombatFxSetup setup);

    // 흔들림·번쩍임 세기 (접근성)
    void set_settings(const FeelSettings& settings);
    // 고정 박자마다 (싸움 계산 바로 뒤): 이번 박자의 맞은 일·사라진 빛 탄을 모아 둬요
    void collect(const FieldCombat& combat);
    // 화면 한 장마다: 모아 둔 일을 연출로 바꾸고, 모든 연출의 시간을 흘려요
    void update(float dt_seconds, float alpha, const FieldCombat& combat, const PlayerMotion& player,
                engine::Vec2 drawn_feet, engine::ParticleRandom& random, engine::ParticlePool& air_fx);
    // 허수아비·몬스터·칼자국·빛 탄을 그리기 목록에 넣어요 (show_boxes면 F4 상자도)
    void queue(const engine::Camera2D& camera, const FieldCombat& combat, const PlayerMotion& player,
               float alpha, bool show_boxes, engine::RenderQueue& queue) const;

    // 조명에 더할 움직이는 빛 (빛 탄, 손끝, 몬스터 공격 예고)
    [[nodiscard]] std::vector<engine::Light> lights(const FieldCombat& combat, float alpha) const;
    // 카메라를 옮길 흔들림 (픽셀)
    [[nodiscard]] engine::Vec2 shake_offset() const { return shake_.offset(); }
    // 떠 있는 피해 숫자 (app 층이 글자로 그려요)
    [[nodiscard]] const DamageNumbers& damage_numbers() const { return numbers_; }

private:
    // 모아 둔 맞은 일 하나를 번쩍임·흔들림·숫자·불꽃으로 바꿔요
    void play_hit(const HitEvent& hit, const FieldCombat& combat, engine::ParticleRandom& random,
                  engine::ParticlePool& air_fx);

    CombatView view_;
    MonsterView monster_view_;
    SlashArc slash_;
    SpellView spell_;
    ScreenShake shake_;
    DamageNumbers numbers_;
    const engine::ParticleEffect* hit_spark_;
    std::vector<HitEvent> pending_hits_;          // 지난 화면 뒤에 생긴 맞은 일
    std::vector<ProjectileEndEvent> pending_ends_;  // 지난 화면 뒤에 사라진 빛 탄
};

}  // namespace wind::game
