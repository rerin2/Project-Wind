// 이 파일은 무엇을 하나요?
// 싸움 연출 담당들을 차례로 갱신하고 그리는 곳이에요. 맞은 일은 번쩍임·흔들림·숫자·불꽃으로 바꿔요.
// 값의 이유는 docs/VISUALS.md "Hit feel" 절에 있어요.

#include "game/presentation/field_combat_fx.hpp"

#include "engine/fx/particle_spawn.hpp"

#include <utility>

namespace wind::game {
namespace {

// 피해 숫자는 맞은 상자 가운데보다 32픽셀 위(머리 위 HP 바 바로 위)에서 시작해요.
// 10·24로 해 보니 허수아비 머리 위 HP 바와 숫자가 겹쳐서 읽기 힘들었어요 (10b 화면 확인).
constexpr float kNumberLift = 32.0f;
constexpr int kCritSparkBursts = 2;   // 치명타는 불꽃을 두 번 터뜨려요

}  // namespace

FieldCombatFx::FieldCombatFx(CombatFxSetup setup)
    : view_(setup.shadow, std::move(setup.dummies)),
      monster_view_(setup.shadow, setup.glow, std::move(setup.monsters)),
      slash_(setup.slash),
      spell_(setup.star_trail, setup.star_burst, setup.glow, setup.spell_light),
      hit_spark_(setup.hit_spark) {}

void FieldCombatFx::set_settings(const FeelSettings& settings) {
    shake_.set_strength(settings.shake);
    view_.set_flash_strength(settings.flash);
}

void FieldCombatFx::collect(const FieldCombat& combat) {
    const auto& hits = combat.hits();
    pending_hits_.insert(pending_hits_.end(), hits.begin(), hits.end());
    const auto& ends = combat.projectile_ends();
    pending_ends_.insert(pending_ends_.end(), ends.begin(), ends.end());
}

void FieldCombatFx::update(float dt_seconds, float alpha, const FieldCombat& combat,
                           const PlayerMotion& player, engine::Vec2 drawn_feet,
                           engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    for (const auto& hit : pending_hits_) {  // 1) 모아 둔 맞은 일을 연출로
        play_hit(hit, combat, random, air_fx);
    }
    pending_hits_.clear();
    // 2) 마술: 시전 중이면 손끝 빛, 빛 탄 꼬리, 사라진 자리 반짝
    std::optional<CastGlow> cast;
    if (player.caster().busy()) {
        cast = CastGlow{FieldCombat::spell_hand(drawn_feet, player.cast_facing()), player.caster().progress()};
    }
    spell_.update(dt_seconds, combat.projectiles(), alpha, pending_ends_, cast, random, air_fx);
    pending_ends_.clear();
    // 3) 나머지 연출의 시간 흘리기: HP 바·번쩍임, 칼자국, 흔들림, 숫자
    view_.update(dt_seconds, combat.dummies());
    monster_view_.update(dt_seconds, combat.monsters());  // 숨쉬기·뜀 세기 (11a)
    slash_.update(dt_seconds, player.attack(), player.attack_hitbox(), player.mover().facing());
    shake_.update(dt_seconds);
    numbers_.update(dt_seconds);
}

void FieldCombatFx::play_hit(const HitEvent& hit, const FieldCombat& combat,
                             engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    view_.flash(combat.dummies(), hit.target);      // 하얀 번쩍임
    shake_.add(hit.shake);                          // 화면 흔들림 (트라우마)
    numbers_.spawn({hit.at.x, hit.at.y - kNumberLift}, hit.amount, hit.critical);  // 피해 숫자
    if (hit_spark_ == nullptr) {
        return;  // 불꽃 효과 파일이 없으면 불꽃만 빼요
    }
    // 불꽃은 밀린 방향으로 튀어요. 치명타는 두 번
    const float degrees = engine::direction_to_degrees(hit.direction);
    const int bursts = hit.critical ? kCritSparkBursts : 1;
    for (int i = 0; i < bursts; ++i) {
        (void)engine::emit_burst(*hit_spark_, hit.at, degrees, random, air_fx);
    }
}

void FieldCombatFx::queue(const engine::Camera2D& camera, const FieldCombat& combat,
                          const PlayerMotion& player, float alpha, bool show_boxes,
                          engine::RenderQueue& queue) const {
    // 허수아비(주인공과 발 위치로 줄 서기), HP 바, 번쩍임, F4면 판정 상자
    view_.queue(camera, combat.dummies(), player.attack_hitbox(), show_boxes, queue);
    monster_view_.queue(camera, combat.monsters(), alpha, show_boxes, queue);  // 몬스터 (11a)
    slash_.queue(camera, queue);                                        // 칼자국
    spell_.queue(camera, combat.projectiles(), alpha, show_boxes, queue);  // 손끝 빛과 빛 탄
}

std::vector<engine::Light> FieldCombatFx::lights(const FieldCombat& combat, float alpha) const {
    auto result = spell_.lights(combat.projectiles(), alpha);         // 빛 탄, 손끝
    const auto warnings = monster_view_.lights(combat.monsters());    // 공격 예고 붉은 빛 (11b)
    result.insert(result.end(), warnings.begin(), warnings.end());
    return result;
}

}  // namespace wind::game
