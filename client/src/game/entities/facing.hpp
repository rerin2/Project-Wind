#pragma once

// 이 파일은 무엇을 하나요?
// "바라보는 방향"(위·아래·왼쪽·오른쪽 4개)과 "화살표"(Vec2)를 서로 바꾸는 도우미예요.
// 캐릭터는 8방향으로 걷지만, 그림은 아직 4방향뿐이라서 가장 가까운 그림을 골라야 해요.

#include "engine/core/math.hpp"

#include "wind/world/direction.hpp"

namespace wind::game {

// 화살표 방향을 4방향 그림 중 하나로 골라요.
// 더 많이 가는 축을 따라요. 정확히 대각선이면 지금 보고 있는 방향을 되도록 유지해요.
// (그래야 대각선으로 걸을 때 그림이 왼쪽↔위로 깜빡깜빡 바뀌지 않아요.)
[[nodiscard]] world::Direction facing_from_vector(engine::Vec2 v, world::Direction current);

// 4방향을 길이 1인 화살표로 바꿔요. 예: Left -> (-1, 0)
[[nodiscard]] engine::Vec2 direction_vector(world::Direction dir);

}  // namespace wind::game
