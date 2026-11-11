#pragma once

// 이 파일은 무엇을 하나요?
// 공격 데이터의 히트박스(오른쪽을 볼 때 기준)를 맵 위의 진짜 네모로 바꿔 주는 도우미예요.
// 규칙 (docs/ENGINE.md "Attack" 절):
//   오른쪽: 발에서 앞(+x)으로 forward, 위아래로 lift. 가로 = length, 세로 = width
//   왼쪽:   앞이 -x (거울처럼 뒤집기)
//   아래/위: 앞이 +y / -y, 가로·세로가 바뀌어요 (가로 = width, 세로 = length)
//   lift는 언제나 화면 위아래로만 옮겨요 (몸통 높이 맞추기).

#include "engine/core/math.hpp"

#include "wind/combat/attack.hpp"
#include "wind/world/direction.hpp"

namespace wind::game {

[[nodiscard]] engine::RectF place_hitbox(const combat::HitboxShape& shape, engine::Vec2 feet,
                                         world::Direction facing);

}  // namespace wind::game
