#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터 공격이 주인공에게 닿았을 때 피해·경직·넉백을 주고 기록을 만드는 도우미 함수예요. (11c, 13c)
// 덮치기(몸으로 부딪힘)와 날아오는 침이 같은 규칙을 써요:
//   피해 = 몬스터 공격력 × 공격 위력 − 주인공 방어력 (명세 9.3), 경직·무적·넉백은 PlayerMotion::take_hit
// 무적인지, 이번 공격에서 이미 맞혔는지는 부르는 쪽이 먼저 확인해요.

#include "engine/core/math.hpp"
#include "game/combat/hero_hit_event.hpp"

#include "wind/combat/attack.hpp"

namespace wind::game {

class PlayerMotion;

// attack_power: 몬스터 공격력, attack: 공격 데이터 (위력·넉백·멈칫·흔들림), direction: 밀려날 쪽 (길이 1)
[[nodiscard]] HeroHitEvent strike_hero(PlayerMotion& player, float attack_power, const combat::AttackData& attack,
                                       engine::Vec2 direction);

}  // namespace wind::game
