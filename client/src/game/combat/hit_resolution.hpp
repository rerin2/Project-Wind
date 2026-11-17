#pragma once

// 이 파일은 무엇을 하나요?
// "휘두른 칼이 허수아비·몬스터에 맞았나?"를 정하는 도우미 함수예요. (명세 9.1, 9.3, 몬스터는 11d)
// 한 박자에 한 번 불러요. 순서:
//   1) 칼의 히트박스와 허수아비의 허트박스가 겹치나? (서 있는 허수아비만)
//   2) 이번 휘두르기에서 처음 맞는 대상인가? (register_hit가 기억해 줘요)
//   3) 피해 계산 순수 함수(compute_damage)로 깎을 양을 정하고 허수아비에 적용해요.
// 맞은 일은 HitEvent로 돌려줘서, 연출(10단계: 번쩍임, 숫자)이 보고 쓸 수 있어요.
// 연출은 이 결과를 "보기만" 해요. SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/combat_random.hpp"
#include "game/combat/training_dummy.hpp"

#include "wind/combat/damage.hpp"

#include <functional>
#include <span>
#include <vector>

namespace wind::game {

class Monster;

// 이번 박자에 판정 중인 공격 한 번 (칼 휘두르기, 또는 빛 탄의 한 걸음)
struct Swing {
    engine::RectF hitbox;       // 공격이 닿는 맵 위 네모
    float attack_power = 0.0f;  // 공격력 (무기 + 힘, 또는 마법 공격력)
    float power = 1.0f;         // 이번 공격의 위력 배수 (콤보마다 달라요)
    float crit_chance = 0.0f;   // 치명타 확률 0~1 (주사위가 있을 때만 굴려요)
    // 타격감 (명세 6.7, 10a): 데이터 값을 그대로 맞은 일에 실어 보내요
    engine::Vec2 push_direction;  // 밀어낼 방향 (길이 1, 칼은 바라보는 쪽, 빛 탄은 나는 쪽)
    float knockback = 0.0f;     // 밀어내는 힘 (픽셀/초)
    float hitstop = 0.0f;       // 맞는 순간 멈칫 (초)
    float shake = 0.0f;         // 화면 흔들림 세기 0~1 (연출만 써요)
    // 이 대상을 이번 휘두르기에서 처음 맞혔으면 true를 돌려주고 기억해요.
    std::function<bool(TargetId)> register_hit;
};

// 맞은 일 하나 (연출과 기록용). 10b의 번쩍임·흔들림·피해 숫자가 이걸 보고 그려요.
struct HitEvent {
    TargetId target = 0;      // 맞은 대상
    int amount = 0;           // 깎인 HP
    engine::Vec2 at;          // 맞은 곳 (허트박스 가운데)
    bool knocked_down = false;  // 이번에 HP가 0이 되어 쓰러졌나
    bool critical = false;    // 치명타였나 (피해 × 1.5)
    engine::Vec2 direction;   // 밀린 방향
    float knockback = 0.0f;   // 밀어낸 힘
    float hitstop = 0.0f;     // 멈칫 시간
    float shake = 0.0f;       // 화면 흔들림 세기
};

// 공격 한 번으로 맞은 허수아비들에게 피해를 주고 밀어내요. 맞은 일 목록을 돌려줘요.
// random: 치명타 주사위 (없으면 치명타 없음). 맞은 대상마다 한 번씩 굴려요.
[[nodiscard]] std::vector<HitEvent> resolve_swing(const Swing& swing,
                                                  std::span<TrainingDummy> dummies,
                                                  CombatRandom* random = nullptr,
                                                  const combat::DamageRules& rules = {});

// 같은 규칙으로 몬스터들에게 (11d). 몬스터는 맞으면 움찔하고 밀려나며, HP 0이면 쓰러져요.
[[nodiscard]] std::vector<HitEvent> resolve_swing(const Swing& swing, std::span<Monster> monsters,
                                                  CombatRandom* random = nullptr,
                                                  const combat::DamageRules& rules = {});

}  // namespace wind::game
