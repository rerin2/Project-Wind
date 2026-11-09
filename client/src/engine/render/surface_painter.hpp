#pragma once

// 이 파일은 무엇을 하나요?
// 파일 없이 코드로 그리는 그림들(체크무늬, 부드러운 타원, 둥근 빛, 꽉 찬 네모)을
// 컴퓨터 기억 속 그림(SDL "서피스")에 픽셀 하나하나 칠해 주는 도우미들이에요.
// 렌더러가 이 그림을 받아서 그래픽 카드용 스티커(텍스처)로 바꿔요.
// 돌려준 서피스는 받은 쪽이 다 쓰고 SDL_DestroySurface로 버려야 해요. 실패하면 nullptr.

#include "engine/render/render_types.hpp"

#include <cstdint>

struct SDL_Surface;

namespace wind::engine {

// 분홍/검정 4x4 체크무늬 (그림 파일이 없을 때 대신 보여 주는 그림)
[[nodiscard]] SDL_Surface* paint_checker(std::int32_t size);

// 가운데가 진하고 가장자리로 갈수록 흐려지는 타원 (그림자, 먼지). color.a = 가운데 진하기
[[nodiscard]] SDL_Surface* paint_soft_ellipse(std::int32_t width, std::int32_t height, Color color);

// 둥근 빛: 흰색, 가운데 진하기 255에서 가장자리 0으로 (1 - 거리²)² 모양으로 사라져요.
[[nodiscard]] SDL_Surface* paint_radial_light(std::int32_t size);

// 한 가지 색으로 꽉 찬 네모
[[nodiscard]] SDL_Surface* paint_solid(std::int32_t width, std::int32_t height, Color color);

}  // namespace wind::engine
