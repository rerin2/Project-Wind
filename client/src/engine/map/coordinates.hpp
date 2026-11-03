#pragma once

// 이 파일은 무엇을 하나요?
// 맵은 바둑판처럼 칸(타일)으로 되어 있어요. 게임 규칙은 "몇 번째 칸"으로 생각하고,
// 그림은 "몇 번째 픽셀"로 그려요. 이 파일은 칸 번호 <-> 픽셀 위치를 바꿔 주는 계산기예요.
// (명세 5.3) 이런 변환은 여기와 카메라(Camera2D) 두 곳에서만 해서 헷갈리지 않게 해요.
//
// 용어: 타일 = 맵 바둑판의 한 칸, 월드 픽셀 = 맵 전체를 큰 그림으로 봤을 때의 픽셀 위치

#include "engine/core/math.hpp"

#include "wind/world/direction.hpp"

#include <cstdint>

namespace wind::engine {

// 칸의 왼쪽 위 모서리가 맵 그림에서 몇 픽셀 위치인지 알려 줘요.
// 예: 칸 크기가 32일 때 (3, 2)번 칸 -> (96, 64)픽셀
[[nodiscard]] Vec2 tile_to_world(world::TileCoord tile, std::int32_t tile_size);

// 칸의 한가운데가 몇 픽셀 위치인지 알려 줘요.
[[nodiscard]] Vec2 tile_center(world::TileCoord tile, std::int32_t tile_size);

// 반대로, 어떤 픽셀 위치가 몇 번째 칸 안에 있는지 알려 줘요.
// 맵 왼쪽 바깥(음수 위치)도 올바르게 계산해요. 예: -1픽셀은 -1번 칸이에요.
[[nodiscard]] world::TileCoord world_to_tile(Vec2 world, std::int32_t tile_size);

}  // namespace wind::engine
