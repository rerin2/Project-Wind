// 이 파일은 무엇을 하나요?
// 칸 번호와 픽셀 위치를 서로 바꾸는 계산을 실제로 하는 곳이에요.

#include "engine/map/coordinates.hpp"

#include <cmath>

namespace wind::engine {

// 칸 번호 x 칸 크기 = 왼쪽 위 픽셀 위치
Vec2 tile_to_world(world::TileCoord tile, std::int32_t tile_size) {
    return {static_cast<float>(tile.x * tile_size), static_cast<float>(tile.y * tile_size)};
}

// 왼쪽 위에서 칸 크기의 절반만큼 오른쪽·아래로 가면 칸의 한가운데예요.
Vec2 tile_center(world::TileCoord tile, std::int32_t tile_size) {
    const float half = static_cast<float>(tile_size) * 0.5f;
    return tile_to_world(tile, tile_size) + Vec2{half, half};
}

world::TileCoord world_to_tile(Vec2 world, std::int32_t tile_size) {
    // 픽셀 위치 ÷ 칸 크기 = 칸 번호. 이때 소수점은 "내림(floor)"으로 버려요.
    // 그냥 정수로 바꾸면 -0.5가 0이 돼서 틀려요. 내림을 하면 -0.5는 -1이 돼서 맞아요.
    const auto size = static_cast<float>(tile_size);
    return {static_cast<std::int32_t>(std::floor(world.x / size)),
            static_cast<std::int32_t>(std::floor(world.y / size))};
}

}  // namespace wind::engine
