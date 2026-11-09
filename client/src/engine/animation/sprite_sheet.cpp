// 이 파일은 무엇을 하나요?
// 그림 표(스프라이트 시트)에서 클립을 찾고, 오려 낼 칸의 위치를 계산하는 곳이에요.

#include "engine/animation/sprite_sheet.hpp"

namespace wind::engine {

// 이름으로 클립을 찾아요.
const AnimationClip* SpriteSheet::clip(std::string_view name) const {
    const auto it = clips.find(name);
    return it != clips.end() ? &it->second : nullptr;
}

// 방향으로 행을, 칸 번호로 열을 골라서 그림 판의 네모 위치를 계산해요.
RectI SpriteSheet::frame_rect(world::Direction dir, int column) const {
    const auto row = direction_rows[static_cast<std::size_t>(dir)];
    return {column * frame_width, row * frame_height, frame_width, frame_height};
}

}  // namespace wind::engine
