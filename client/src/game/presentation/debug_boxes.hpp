#pragma once

// 이 파일은 무엇을 하나요?
// F4 디버그 상자를 그리기 목록에 넣는 도우미 함수 모음이에요. (FieldState에서 옮겨 왔어요, 13c)
// 맵 위의 네모를 화면 위치로 바꿔 반투명 색으로 덮어요. 게임 결과는 바꾸지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"

namespace wind::game {

// world_box: 맵 위 네모 (픽셀), color: 덮을 색 (보통 반투명)
void queue_debug_box(const engine::Camera2D& camera, const engine::RectF& world_box, engine::Color color,
                     engine::RenderQueue& queue);

}  // namespace wind::game
