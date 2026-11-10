// 이 파일은 무엇을 하나요?
// 맵의 칸들을 그림 순서표에 적어 넣는 일을 실제로 하는 곳이에요.

#include "engine/map/tilemap_renderer.hpp"

#include "engine/map/coordinates.hpp"
#include "engine/map/tile_animation.hpp"

#include <algorithm>

namespace wind::engine {

// 맵 레이어 종류 -> 그리는 순서 단계
DrawLayer draw_layer_for(world::RenderLayer layer) {
    switch (layer) {
        case world::RenderLayer::Ground: return DrawLayer::Ground;
        case world::RenderLayer::GroundDecor: return DrawLayer::GroundDecor;
        // 나무 밑동·바위 같은 낮은 물건은 캐릭터와 같은 단계에 넣고,
        // "발이 더 아래에 있는 쪽이 앞에 보이게" 순서를 정해요.
        case world::RenderLayer::LowObjects: return DrawLayer::YSorted;
        case world::RenderLayer::HighObjects: return DrawLayer::HighObjects;
        case world::RenderLayer::Foreground: return DrawLayer::Foreground;
    }
    return DrawLayer::Ground;
}

RectI tile_source_rect(const world::Tileset& tileset, world::TileId tile) {
    // 0번은 "빈 칸"이라 그림이 1번부터 시작해요. 그래서 1을 빼서 0부터 세요.
    const int index = static_cast<int>(tile) - 1;
    // 한 줄에 columns개씩 있으니, 나머지가 몇 번째 열(가로), 몫이 몇 번째 줄(세로)이에요.
    const int col = index % tileset.columns;
    const int row = index / tileset.columns;
    return {col * tileset.tile_width, row * tileset.tile_height, tileset.tile_width,
            tileset.tile_height};
}

TileRange visible_tiles(const world::MapData& map, const Camera2D& camera) {
    // 카메라가 보고 있는 네모의 왼쪽 위 칸과 오른쪽 아래 칸을 구해요.
    const RectF view = camera.view_rect();
    const world::TileCoord first = world_to_tile({view.x, view.y}, map.tile_size);
    const world::TileCoord last = world_to_tile({view.right(), view.bottom()}, map.tile_size);
    // 마지막 칸은 반쯤 걸쳐 보일 수 있어서 한 칸 더(+1) 포함해요.
    // clamp(자르기)로 맵 밖 칸 번호가 나오지 않게 해요.
    return {{std::clamp(first.x, 0, map.width), std::clamp(first.y, 0, map.height)},
            {std::clamp(last.x + 1, 0, map.width), std::clamp(last.y + 1, 0, map.height)}};
}

std::uint32_t queue_tilemap(const world::MapData& map, const world::Tileset& tileset,
                            TextureHandle texture, const Camera2D& camera, float time_seconds,
                            RenderQueue& queue) {
    const TileRange range = visible_tiles(map, camera);
    const auto size = static_cast<float>(map.tile_size);
    std::uint32_t count = 0;

    // 레이어마다, 보이는 칸마다 하나씩 살펴봐요.
    for (const auto& layer : map.layers) {
        const DrawLayer draw_layer = draw_layer_for(layer.render_layer);
        for (std::int32_t y = range.min.y; y < range.max.y; ++y) {
            for (std::int32_t x = range.min.x; x < range.max.x; ++x) {
                const world::TileId tile = map.tile_at(layer, {x, y});
                if (tile == world::kEmptyTile) {
                    continue;  // 빈 칸은 그릴 게 없어요
                }
                // 맵 위 위치 -> 화면 위 위치로 바꿔서 어디에 그릴지 정해요.
                const Vec2 world_pos = tile_to_world({x, y}, map.tile_size);
                const Vec2 screen = camera.world_to_screen(world_pos);
                // 움직이는 카드면 지금 보여 줄 그림으로 바꾸고, 흔들리는 카드면 좌우로 조금 밀어요.
                world::TileId shown = tile;
                float sway = 0.0f;
                if (const auto* props = tileset.find(tile)) {
                    shown = animated_tile(*props, time_seconds, {x, y});
                    if (props->sway) {
                        sway = sway_offset(*props->sway, time_seconds, {x, y});
                    }
                }
                SpriteDraw draw;
                draw.texture = texture;
                draw.source = tile_source_rect(tileset, shown);
                draw.dest = {screen.x + sway, screen.y, size, size};
                // 순서를 정하는 기준은 칸의 아래쪽 끝 (물건이 땅에 닿는 곳)
                queue.push_sprite(draw_layer, world_pos.y + size, draw);
                ++count;
            }
        }
    }
    return count;
}

void queue_collision_overlay(const world::MapData& map, const Camera2D& camera, RenderQueue& queue) {
    const TileRange range = visible_tiles(map, camera);
    const auto size = static_cast<float>(map.tile_size);
    // 빨강(220,40,40)에 투명도 110 (255가 불투명, 0이 완전 투명)
    constexpr Color kBlocked{220, 40, 40, 110};

    for (std::int32_t y = range.min.y; y < range.max.y; ++y) {
        for (std::int32_t x = range.min.x; x < range.max.x; ++x) {
            if (map.is_walkable({x, y})) {
                continue;  // 지나갈 수 있는 칸은 표시하지 않아요
            }
            const Vec2 screen = camera.world_to_screen(tile_to_world({x, y}, map.tile_size));
            queue.push_fill(DrawLayer::DebugOverlay, 0.0f, {screen.x, screen.y, size, size}, kBlocked);
        }
    }
}

}  // namespace wind::engine
