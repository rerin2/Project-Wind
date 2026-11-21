#pragma once

// 이 파일은 무엇을 하나요?
// 그림 픽셀을 흑백 쪽으로 바꾸는 도우미 함수예요. (명세 9.5, 12b: 쓰러지면 화면이 흑백으로)
// 픽셀은 한 줄에 R, G, B, A 바이트 순서로 놓여 있어야 해요 (SDL_PIXELFORMAT_RGBA32).
// 밝기는 사람 눈 기준 무게(빨강 30%, 초록 59%, 파랑 11%)로 구해요.
// SDL을 몰라서 창 없이 테스트할 수 있어요. 화면에서 읽고 다시 올리는 일은 frame_readback이 해요.

#include <cstdint>
#include <span>

namespace wind::engine {

// pixels: RGBA 바이트들 (4의 배수), amount: 0이면 그대로, 1이면 완전 흑백. 알파(A)는 그대로 둬요.
void desaturate_rgba(std::span<std::uint8_t> pixels, float amount);

}  // namespace wind::engine
