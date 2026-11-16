#pragma once

// 이 파일은 무엇을 하나요?
// "휘두른 칼이 허수아비에 맞았나?"를 정하는 도우미 함수예요. (명세 9.1, 9.3)
// 한 박자에 한 번 불러요. 순서:
//   1) 칼의 히트박스와 허수아비의 허트박스가 겹치나? (서 있는 허수아비만)
//   2) 이번 휘두르기에서 처음 맞는 대상인가? (register_hit가 기억해 줘요)
//   3) 피해 계산 순수 함수(compute_damage)로 깎을 양을 정하고 허수아비에 적용해요.
// 맞은 일은 HitEvent로 돌려줘서, 연출(10단계: 번쩍임, 숫자)이 보고 쓸 수 있어요.
// 연출은 이 결과를 "보기만" 해요. SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/training_dummy.hpp"

#include "wind/combat/damage.hpp"

#include <functional>
#include <span>
#include <vector>

namespace wind::game {

// 이번 박자에 판정 중인 칼 한 번
struct Swing {
    engine::RectF hitbox;       // 칼이 닿는 맵 위 네모
    float attack_power = 0.0f;  // 공격력 (무기 + 힘)
    float power = 1.0f;         // 이번 공격의 위력 배수 (콤보마다 달라요)
    // 이 대상을 이번 휘두르기에서 처음 맞혔으면 true를 돌려주고 기억해요.
    std::function<bool(TargetId)> register_hit;
};

// 맞은 일 하나 (연출과 기록용)
struct HitEvent {
    TargetId target = 0;      // 맞은 대상
    int amount = 0;           // 깎인 HP
    engine::Vec2 at;          // 맞은 곳 (허트박스 가운데)
    bool knocked_down = false;  // 이번에 HP가 0이 되어 쓰러졌나
};

// 칼 한 번으로 맞은 허수아비들에게 피해를 주고, 맞은 일 목록을 돌려줘요.
[[nodiscard]] std::vector<HitEvent> resolve_swing(const Swing& swing,
                                                  std::span<TrainingDummy> dummies,
                                                  const combat::DamageRules& rules = {});

}  // namespace wind::game
