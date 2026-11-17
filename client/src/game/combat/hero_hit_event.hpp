#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 맞은 일 하나예요. (11c) FieldCombat이 적고, 히트스톱·연출(흔들림·붉은 비네트)이 보고 써요.
// 연출은 이 값을 "보기만" 해요.

#include "engine/core/math.hpp"

namespace wind::game {

struct HeroHitEvent {
    int amount = 0;           // 깎인 HP (HP가 1이면 0일 수도 있어요, 11c 임시 규칙)
    engine::Vec2 at;          // 맞은 곳 (주인공 맞는 상자 가운데)
    engine::Vec2 direction;   // 밀린 방향 (길이 1, 덮친 방향)
    float knockback = 0.0f;   // 밀어낸 힘 (픽셀/초)
    float hitstop = 0.0f;     // 멈칫 시간 (초)
    float shake = 0.0f;       // 화면 흔들림 세기 0~1
};

}  // namespace wind::game
