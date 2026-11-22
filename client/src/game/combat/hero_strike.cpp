// 이 파일은 무엇을 하나요?
// 주인공 맞히기: 피해 계산 -> HP·경직·넉백 -> 연출용 기록.

#include "game/combat/hero_strike.hpp"

#include "game/entities/player_motion.hpp"

#include "wind/combat/damage.hpp"

namespace wind::game {
namespace {

// 주인공 방어력 (명세 8.2·10.3). 장비가 생기는 15단계 전까지는 0이에요.
constexpr float kHeroDefense = 0.0f;

}  // namespace

HeroHitEvent strike_hero(PlayerMotion& player, float attack_power, const combat::AttackData& attack,
                         engine::Vec2 direction) {
    combat::DamageInput input;                    // 피해 계산 (명세 9.3)
    input.attack_power = attack_power;            // 몬스터 공격력
    input.power = attack.power;                   // 공격 위력
    input.defense = kHeroDefense;
    const combat::DamageResult damage = combat::compute_damage(input);
    const engine::RectF hurtbox = player.hurtbox();  // 맞기 전 자리 (연출이 여기서 터져요)
    HeroHitEvent hit;
    hit.amount = player.take_hit(damage.amount, direction, attack.knockback);  // HP·경직·무적·넉백
    hit.fatal = player.dead();                    // 이 한 방에 쓰러졌나 (12b)
    hit.at = {hurtbox.x + hurtbox.w * 0.5f, hurtbox.y + hurtbox.h * 0.5f};
    hit.direction = direction;
    hit.knockback = attack.knockback;
    hit.hitstop = attack.hitstop;
    hit.shake = attack.shake;
    return hit;
}

}  // namespace wind::game
