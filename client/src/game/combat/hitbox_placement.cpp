// 이 파일은 무엇을 하나요?
// 히트박스를 바라보는 방향에 맞게 돌려서 맵 위 네모로 만들어요.

#include "game/combat/hitbox_placement.hpp"

namespace wind::game {

engine::RectF place_hitbox(const combat::HitboxShape& shape, engine::Vec2 feet,
                           world::Direction facing) {
    engine::Vec2 center = feet;
    float w = shape.length;
    float h = shape.width;
    switch (facing) {
        case world::Direction::Right: center.x += shape.forward; break;
        case world::Direction::Left: center.x -= shape.forward; break;
        case world::Direction::Down:
            center.y += shape.forward;
            w = shape.width;
            h = shape.length;
            break;
        case world::Direction::Up:
            center.y -= shape.forward;
            w = shape.width;
            h = shape.length;
            break;
    }
    center.y += shape.lift;
    return {center.x - w * 0.5f, center.y - h * 0.5f, w, h};
}

}  // namespace wind::game
