#pragma once

// 이 파일은 무엇을 하나요?
// 저장 아이콘을 그려요. (명세 12.3, 16c, 개발자와 정함: 오른쪽 위, 코드로 그린 그림)
// 금빛 캣닢 잎 하나(점 그림을 2배로)가 위아래로 살짝 흔들리고, 작은 반짝이가 잎 둘레를 돌아요.
// 언제·얼마나 진하게 보일지는 game::SaveIcon이 정해요. 그림 파일이 없어서 빠질 일이 없어요.

#include "engine/core/math.hpp"

namespace wind::engine {
class Renderer;
}
namespace wind::game {
class SaveIcon;
}

namespace wind::client {

// screen: 화면 크기 (논리 픽셀)
void draw_save_icon(engine::Renderer& renderer, engine::Vec2 screen, const game::SaveIcon& icon);

}  // namespace wind::client
