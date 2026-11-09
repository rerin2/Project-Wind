// 이 파일은 무엇을 하나요?
// 방향 ↔ 화살표 바꾸기를 실제로 계산하는 곳이에요.

#include "game/entities/facing.hpp"

#include <cmath>

namespace wind::game {

using world::Direction;

Direction facing_from_vector(engine::Vec2 v, Direction current) {
    const float ax = std::abs(v.x);
    const float ay = std::abs(v.y);
    if (ax == 0.0f && ay == 0.0f) {
        return current;  // 화살표가 없으면 그대로
    }
    const Direction horizontal = v.x < 0.0f ? Direction::Left : Direction::Right;
    const Direction vertical = v.y < 0.0f ? Direction::Up : Direction::Down;
    if (ax > ay) {
        return horizontal;
    }
    if (ay > ax) {
        return vertical;
    }
    // 정확히 대각선: 지금 보는 방향이 둘 중 하나면 그대로, 아니면 옆모습(가로)
    if (current == horizontal || current == vertical) {
        return current;
    }
    return horizontal;
}

engine::Vec2 direction_vector(Direction dir) {
    switch (dir) {
        case Direction::Up: return {0.0f, -1.0f};
        case Direction::Down: return {0.0f, 1.0f};
        case Direction::Left: return {-1.0f, 0.0f};
        case Direction::Right: return {1.0f, 0.0f};
    }
    return {0.0f, 1.0f};
}

}  // namespace wind::game
