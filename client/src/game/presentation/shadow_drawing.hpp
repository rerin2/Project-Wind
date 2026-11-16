#pragma once

// 이 파일은 무엇을 하나요?
// 발밑 그림자를 그리기 목록에 넣는 도우미 함수예요.
// 주인공과 허수아비가 같은 둥근 그림자 그림을 크기만 다르게 써요.

#include "engine/camera/camera2d.hpp"
#include "engine/core/math.hpp"
#include "engine/render/render_queue.hpp"

namespace wind::game {

// feet(맵 위 발 위치)를 가운데로 size 크기 그림자를 Shadows 단계에 놓아요. 그림이 없으면 안 그려요.
void queue_shadow(const engine::Camera2D& camera, engine::TextureHandle texture, engine::Vec2 feet,
                  engine::Vec2 size, engine::RenderQueue& queue);

}  // namespace wind::game
