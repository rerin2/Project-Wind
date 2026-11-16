// 이 파일은 무엇을 하나요?
// 발밑 그림자 하나를 그리기 목록에 넣어요. (field_state, combat_view에서 옮겨 와 하나로 합쳤어요)

#include "game/presentation/shadow_drawing.hpp"

#include <cmath>

namespace wind::game {

void queue_shadow(const engine::Camera2D& camera, engine::TextureHandle texture, engine::Vec2 feet,
                  engine::Vec2 size, engine::RenderQueue& queue) {
    if (!texture.valid()) {
        return;  // 그림이 없으면 그림자 없이 그려요
    }
    const engine::Vec2 screen = camera.world_to_screen(feet);
    engine::SpriteDraw shadow;
    shadow.texture = texture;
    // 발 위치가 그림자 한가운데. 픽셀에 딱 맞게 반올림해요
    shadow.dest = {std::round(screen.x - size.x * 0.5f), std::round(screen.y - size.y * 0.5f), size.x,
                   size.y};
    queue.push_sprite(engine::DrawLayer::Shadows, feet.y, shadow);  // 발 위치(y)로 줄 서기
}

}  // namespace wind::game
