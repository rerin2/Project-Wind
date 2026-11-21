#pragma once

// 이 파일은 무엇을 하나요?
// 지금 그리고 있는 도화지(렌더 타깃)의 픽셀을 그래픽 카드에서 컴퓨터 기억(CPU)으로 읽어 오는 도우미 함수들이에요.
// - 스크린샷: 읽어서 BMP 파일로 저장 (F12)
// - 흑백: 읽어서 회색 쪽으로 바꾼 다음, 다시 그래픽 카드에 올려 도화지 위에 덮어 그려요 (12b, 개발자와 정함)
// 읽어 오기는 느려서(640x360 한 장에 몇 밀리초) 흑백은 주인공이 쓰러져 있을 때만 해요.
// Renderer만 써요. SDL 함수를 부르므로 SDL 층(engine/render)에만 있어요.

#include <filesystem>

struct SDL_Renderer;
struct SDL_Texture;

namespace wind::engine {

// 지금 도화지를 path에 BMP로 저장해요. 결과(성공·실패 이유)는 기록(로그)에 남겨요.
void save_frame_bmp(SDL_Renderer* renderer, const std::filesystem::path& path);

// 지금 도화지를 amount(0~1)만큼 흑백으로 바꿔요. scratch: 도화지와 같은 크기의 RGBA32 스트리밍 텍스처.
// 실패하면 기록을 남기고 false (화면은 색 그대로예요).
bool desaturate_frame(SDL_Renderer* renderer, SDL_Texture* scratch, float amount);

}  // namespace wind::engine
