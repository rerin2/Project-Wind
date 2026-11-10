#pragma once

// 이 파일은 무엇을 하나요?
// 움직이는 타일과 바람에 흔들리는 타일을 "지금 몇 초"에 맞춰 계산하는 도우미 함수들이에요. (명세 7.1)
// - animated_tile: 물·꽃처럼 그림 카드를 바꿔 보여 줄 때, 지금 보여 줄 카드 번호
// - sway_offset:   풀·나뭇잎이 바람에 좌우로 몇 픽셀 밀렸는지
// 칸 위치마다 박자를 조금씩 어긋나게 해서, 모든 칸이 한꺼번에 똑같이 움직이지 않아요.
// 바람은 왼쪽에서 오른쪽으로 물결처럼 지나가요.
// 보여 주기 전용이고, 같은 시각·같은 칸이면 언제나 같은 답이 나와요 (주사위를 쓰지 않아요).

#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

namespace wind::engine {

// tile 카드가 움직이는 카드라면, 지금(time_seconds) 이 칸(cell)에 보여 줄 카드 번호.
// 움직이지 않는 카드면 원래 번호를 그대로 돌려줘요.
[[nodiscard]] world::TileId animated_tile(const world::TileProperties& tile, float time_seconds,
                                          world::TileCoord cell);

// 지금 이 칸의 흔들림: 좌우로 몇 픽셀 밀렸는지 (도트가 흐려지지 않게 정수 픽셀로 반올림)
[[nodiscard]] float sway_offset(const world::TileSway& sway, float time_seconds,
                                world::TileCoord cell);

}  // namespace wind::engine
