// 이 파일은 무엇을 하나요?
// 맵 칸을 그리기 목록에 넣는 장치가 맞게 동작하는지 숙제 검사(테스트)를 해요.
// - 타일 번호로 스티커 판(타일셋 그림)의 맞는 자리를 찾나
// - 맵 층(바닥, 낮은 물건, 높은 물건)이 맞는 그리기 순서로 가나
// - 화면에 보이는 칸만 골라서 넣나 (안 보이는 칸까지 그리면 낭비)
#include "engine/map/tilemap_renderer.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Camera2D;
using wind::engine::DrawLayer;
using wind::engine::RectI;
using wind::engine::RenderQueue;
using wind::world::RenderLayer;

// 검사용 타일셋: 32x32 칸이 한 줄에 4개씩 있는 스티커 판
wind::world::Tileset make_tileset() {
    wind::world::Tileset t;
    t.id = "t";
    t.tile_width = 32;
    t.tile_height = 32;
    t.columns = 4;
    t.tiles = {{1, "a", true}, {6, "b", false}};
    return t;
}

// 검사용 맵: 가로 width칸 x 세로 height칸, 모든 칸이 1번 타일이고 다 지나갈 수 있어요.
wind::world::MapData make_map(std::int32_t width, std::int32_t height) {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = width;
    map.height = height;
    map.layers.push_back({"ground", RenderLayer::Ground,
                          std::vector<wind::world::TileId>(static_cast<std::size_t>(width * height), 1)});
    map.walkable.assign(static_cast<std::size_t>(width * height), 1);
    return map;
}

// 1번 타일은 스티커 판의 첫 칸(0, 0)이에요.
// 6번 타일은 한 줄에 4개씩이니 둘째 줄의 둘째 칸 -> (32, 32)이어야 해요.
TEST(TilemapRenderer, SourceRectFollowsTilesetGrid) {
    const auto tileset = make_tileset();
    EXPECT_EQ(wind::engine::tile_source_rect(tileset, 1), (RectI{0, 0, 32, 32}));
    EXPECT_EQ(wind::engine::tile_source_rect(tileset, 6), (RectI{32, 32, 32, 32}));
}

// 낮은 물건(나무 밑동)은 캐릭터와 같은 층에서 발 위치로 줄을 서야 하고,
// 높은 물건(나뭇잎)은 캐릭터보다 위 층이어야 해요.
TEST(TilemapRenderer, LowObjectsAreYSortedWithEntities) {
    EXPECT_EQ(wind::engine::draw_layer_for(RenderLayer::Ground), DrawLayer::Ground);
    EXPECT_EQ(wind::engine::draw_layer_for(RenderLayer::LowObjects), DrawLayer::YSorted);
    EXPECT_EQ(wind::engine::draw_layer_for(RenderLayer::HighObjects), DrawLayer::HighObjects);
}

// 맵이 100x100칸(1만 칸)이어도, 640x360 화면에 보이는 약 250칸만 넣어야 해요.
TEST(TilemapRenderer, QueuesOnlyVisibleTiles) {
    const auto map = make_map(100, 100);
    Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.set_world_bounds({0.0f, 0.0f, 3200.0f, 3200.0f});
    camera.center_on({0.0f, 0.0f});  // 왼쪽 위 모서리

    RenderQueue queue;
    const auto count = wind::engine::queue_tilemap(map, make_tileset(), {1}, camera, queue);
    // 화면 640x360 = 20x11.25칸 -> 경계 포함 최대 21x12칸
    EXPECT_LE(count, 21u * 12u);
    EXPECT_GE(count, 20u * 11u);
    EXPECT_EQ(queue.commands().size(), count);
}

// 맵(5x5칸)이 화면보다 작아도, 보이는 칸 범위는 맵 밖(음수나 5 이상)으로 나가면 안 돼요.
TEST(TilemapRenderer, VisibleRangeIsClampedToMap) {
    const auto map = make_map(5, 5);
    Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.set_world_bounds({0.0f, 0.0f, 160.0f, 160.0f});
    camera.center_on({80.0f, 80.0f});
    const auto range = wind::engine::visible_tiles(map, camera);
    EXPECT_EQ(range.min, (wind::world::TileCoord{0, 0}));
    EXPECT_EQ(range.max, (wind::world::TileCoord{5, 5}));
}

}  // namespace
