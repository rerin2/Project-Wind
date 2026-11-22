#pragma once

// 이 파일은 무엇을 하나요?
// 필드 싸움 연출을 한데 모은 담당이에요. (9c, 9e, 10b, 11a) 보여 주기만 하고 게임 결과는 바꾸지 않아요.
// - 허수아비 그림·HP 바·번쩍임 (CombatView)
// - 몬스터 그림: 숨쉬기·통통 뛰기, 느낌표, 공격 예고 바닥 표시 (MonsterView, 11a·11b)
// - 칼자국 (SlashArc), 손끝 빛·빛 탄·꼬리 (SpellView)
// - 타격감 (10b): 화면 흔들림(ScreenShake), 피해 숫자(DamageNumbers), 타격 불꽃(hit_spark 파티클)
// - 주인공이 맞을 때 (11c): 화면 흔들림과 화면 가장자리 붉은 비네트(HurtVignette). 깜빡임은 PlayerSprite가 그려요
// - 몬스터가 맞을 때·사라질 때 (11d): 번쩍임·숫자·불꽃은 허수아비와 같고, 사라지는 순간 death_fx 파티클이 터져요
// - 몬스터를 쓰러뜨리면 (13c): 금빛 룬 알갱이가 튀어나와 주인공에게 빨려 들어가요 (RuneOrbs)
// - 몬스터가 쏜 침 (13c): 초록 방울로 날아가고, 사라질 때 초록 방울이 톡 터져요
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
#include "game/presentation/hurt_vignette.hpp"
#include "game/presentation/monster_view.hpp"
#include "game/presentation/rune_orbs.hpp"
#include "game/presentation/screen_shake.hpp"
#include "game/presentation/slash_arc.hpp"
#include "game/presentation/spell_view.hpp"
#include "game/states/field_combat.hpp"

#include "wind/combat/spell.hpp"

#include <map>
#include <optional>
#include <string>
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
    engine::TextureHandle vignette;                     // 가장자리가 진한 비네트 그림 (11c)
    const std::map<std::string, engine::ParticleEffect>* monster_fx = nullptr;  // 몬스터가 사라질 때 효과 (11d)
    const engine::ParticleEffect* spit_splash = nullptr;  // 몬스터 침이 사라질 때 톡 (13c)
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
    // 다 사라진 몬스터 자리에서 사라짐 파티클을 터뜨려요 (11d)
    void play_vanish(const MonsterVanishEvent& vanish, const FieldCombat& combat, engine::ParticleRandom& random,
                     engine::ParticlePool& air_fx) const;
    // 모아 둔 맞은 일 하나를 번쩍임·흔들림·숫자·불꽃으로 바꿔요
    void play_hit(const HitEvent& hit, const FieldCombat& combat, engine::ParticleRandom& random,
                  engine::ParticlePool& air_fx);
    // 몬스터가 쏜 침: 초록 방울 + 밝은 심 (13c)
    void queue_enemy_shots(const engine::Camera2D& camera, const FieldCombat& combat, float alpha,
                           engine::RenderQueue& queue) const;

    CombatView view_;
    MonsterView monster_view_;
    SlashArc slash_;
    SpellView spell_;
    ScreenShake shake_;
    HurtVignette vignette_;                       // 주인공 피격 붉은 가장자리 (11c)
    engine::TextureHandle vignette_texture_;      // 비네트 그림
    DamageNumbers numbers_;
    const engine::ParticleEffect* hit_spark_;
    std::vector<HitEvent> pending_hits_;          // 지난 화면 뒤에 생긴 맞은 일
    std::vector<ProjectileEndEvent> pending_ends_;  // 지난 화면 뒤에 사라진 빛 탄
    std::vector<HeroHitEvent> pending_hero_hits_;   // 지난 화면 뒤에 주인공이 맞은 일 (11c)
    std::vector<MonsterVanishEvent> pending_vanished_;  // 지난 화면 뒤에 다 사라진 몬스터 (11d)
    const std::map<std::string, engine::ParticleEffect>* monster_fx_;  // 몬스터 사라짐 효과 (없으면 nullptr)
    std::vector<RuneDropEvent> pending_drops_;    // 지난 화면 뒤에 룬이 나온 자리 (13c)
    RuneOrbs runes_;                              // 빨려 드는 룬 알갱이 (13c)
    engine::TextureHandle glow_;                  // 둥근 빛 그림 (룬 알갱이)
    const engine::ParticleEffect* spit_splash_;   // 침이 터질 때 (13c)
    std::vector<ProjectileEndEvent> pending_spit_ends_;  // 지난 화면 뒤에 사라진 침 (13c)
};

}  // namespace wind::game
