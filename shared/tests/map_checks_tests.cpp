// 이 파일은 무엇을 하나요?
// 맵 마무리 검사(map_checks)를 숙제 검사(테스트)해요.
// - 허수아비가 놓인 칸은 못 지나가는 칸이 돼요 (9c)
// - 벽 위에 놓은 허수아비, 벽 위 등장 자리, 다 막힌 출구는 오류예요
// - 저장소의 진짜 초원 맵: 허수아비 칸이 막혀 있어요
#include "wind/content/map_checks.hpp"
#include "wind/content/map_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using wind::content::ErrorList;
using wind::world::TileCoord;

bool has_error_at(const ErrorList& errors, std::string_view field) {
    return std::ranges::any_of(errors.items(), [field](const auto& e) { return e.field == field; });
}

// 1번 = 풀(지나감), 2번 = 벽
wind::world::Tileset make_tileset() {
    wind::world::Tileset t;
    t.id = "t";
    t.tiles = {{1, "grass", true, {}, {}}, {2, "wall", false, {}, {}}};
    return t;
}

// 4x1 맵: 풀 풀 풀 벽
wind::world::MapData make_map() {
    wind::world::MapData map;
    map.width = 4;
    map.height = 1;
    map.layers.push_back({"ground", wind::world::RenderLayer::Ground, {1, 1, 1, 2}});
    map.spawn_points.push_back({"default", {0, 0}});
    return map;
}

TEST(MapChecks, DummyBlocksItsTile) {
    auto map = make_map();
    map.entities.push_back({wind::world::EntityType::TrainingDummy, "d", {2, 0}, "props/x.json"});
    ErrorList errors;
    wind::content::finish_map_checks(map, make_tileset(), "m.json", errors);
    EXPECT_TRUE(errors.empty());
    EXPECT_TRUE(map.is_walkable({1, 0}));
    EXPECT_FALSE(map.is_walkable({2, 0}));  // 허수아비 칸
}

TEST(MapChecks, ReportsBlockedPlacements) {
    auto map = make_map();
    map.spawn_points.push_back({"bad", {3, 0}});                                                 // 벽 위 등장 자리
    map.entities.push_back({wind::world::EntityType::TrainingDummy, "d", {3, 0}, "props/x.json"});  // 벽 위 허수아비
    wind::world::MapExit exit;
    exit.area_min = exit.area_max = {3, 0};  // 벽뿐인 출구
    map.exits.push_back(exit);
    ErrorList errors;
    wind::content::finish_map_checks(map, make_tileset(), "m.json", errors);
    EXPECT_TRUE(has_error_at(errors, "spawn_points[1]"));
    EXPECT_TRUE(has_error_at(errors, "entities[0]"));
    EXPECT_TRUE(has_error_at(errors, "exits[0]"));
}

// 진짜 초원 맵: 허수아비가 있고, 그 칸은 막혀 있어요.
TEST(MapChecks, ShippedMeadowHasBlockingDummies) {
    const auto result = wind::content::load_map_file(WIND_TEST_CONTENT_DIR, "maps/test_meadow.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& map = result.value->map;
    ASSERT_FALSE(map.entities.empty());
    for (const auto& entity : map.entities) {
        EXPECT_FALSE(map.is_walkable(entity.tile)) << entity.id;
    }
}

}  // namespace
