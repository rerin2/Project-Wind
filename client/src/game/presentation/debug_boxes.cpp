// 이 파일은 무엇을 하나요?
// F4 디버그 상자 그리기 (맵 네모 -> 화면 네모).

#include "game/presentation/debug_boxes.hpp"

namespace wind::game {

void queue_debug_box(const engine::Camera2D& camera, const engine::RectF& world_box, engine::Color color,
                     engine::RenderQueue& queue) {
    const engine::Vec2 top_left = camera.world_to_screen({world_box.x, world_box.y});  // 왼쪽 위만 옮기면 돼요
    queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, {top_left.x, top_left.y, world_box.w, world_box.h}, color);
}

}  // namespace wind::game
