#pragma once

// 이 파일은 무엇을 하나요?
// 맵 바둑판의 칸들을 "그림 그릴 순서표(렌더 큐)"에 적어 넣는 일을 해요.
// 맵 전체를 다 그리면 낭비라서, 지금 화면(카메라)에 보이는 칸만 골라서 적어요.
// 화면 밖 칸을 빼는 것을 컬링(culling, 솎아 내기)이라고 해요.

#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"

#include "wind/world/map.hpp"

#include <cstdint>

namespace wind::engine {

// 맵 레이어(바닥, 장식, 낮은 물건, 높은 물건...)가 몇 번째 순서로 그려지는지 알려 줘요.
[[nodiscard]] DrawLayer draw_layer_for(world::RenderLayer layer);

// 타일 번호에 해당하는 그림이 타일셋 그림 파일의 어디에 있는지 알려 줘요.
// 타일셋은 스티커 판처럼 작은 그림이 줄지어 있어요. 1번이 왼쪽 위 첫 스티커예요.
[[nodiscard]] RectI tile_source_rect(const world::Tileset& tileset, world::TileId tile);

// 화면에 보이는 칸의 범위. min은 포함하고 max는 포함하지 않아요. [min, max)
// 맵 밖으로 나가지 않도록 잘라서 알려 줘요.
struct TileRange {
    world::TileCoord min;
    world::TileCoord max;
};
[[nodiscard]] TileRange visible_tiles(const world::MapData& map, const Camera2D& camera);

// 보이는 칸의 타일 그림들을 순서표에 넣고, 몇 개 넣었는지 알려 줘요.
std::uint32_t queue_tilemap(const world::MapData& map, const world::Tileset& tileset,
                            TextureHandle texture, const Camera2D& camera, RenderQueue& queue);

// 못 지나가는 칸 위에 반투명 빨간 네모를 덮어서 보여 줘요. (F4 키, 명세 5.10)
void queue_collision_overlay(const world::MapData& map, const Camera2D& camera, RenderQueue& queue);

}  // namespace wind::engine
