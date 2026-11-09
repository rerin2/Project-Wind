// 이 파일은 무엇을 하나요?
// 맵·타일셋 설명서 숙제 검사(map_loader)가 제대로 채점하는지 시험하는 시험지예요.
// 작은 연습용 맵을 코드로 직접 만들어 놓고, 일부러 틀리게 고쳐서 잘 잡아내는지 봐요.
// 마지막에는 저장소에 들어 있는 진짜 시험 맵도 검사를 통과하는지 확인해요.
#include "wind/content/map_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

using nlohmann::json;
using wind::world::TileCoord;

// 연습용 타일셋: 1번 = 풀(지나갈 수 있음), 2번 = 벽(못 감), 3번 = 나뭇잎(지나갈 수 있음)
json make_tileset() {
    return json{{"schema_version", 1},
                {"id", "test"},
                {"image", "tilesets/test.bmp"},
                {"tile_width", 32},
                {"tile_height", 32},
                {"columns", 4},
                {"tiles",
                 json::array({json{{"id", 1}, {"name", "grass"}, {"walkable", true}},
                              json{{"id", 2}, {"name", "wall"}, {"walkable", false}},
                              json{{"id", 3}, {"name", "canopy"}, {"walkable", true}}})}};
}

// 연습용 맵: 가로 3칸 x 세로 2칸
//   바닥 필름: 오른쪽 아래 칸만 비어 있고 나머지는 풀
//   물건 필름: 위 줄 가운데에 벽, 아래 줄 가운데에 나뭇잎
json make_map() {
    return json{{"schema_version", 1},
                {"map_id", "unit"},
                {"display_name", "Unit Map"},
                {"tileset", "tilesets/test.json"},
                {"tile_size", 32},
                {"width", 3},
                {"height", 2},
                {"spawn_points", json::array({json{{"id", "default"}, {"x", 0}, {"y", 0}}})},
                {"layers",
                 json::array({json{{"name", "ground"},
                                   {"render_layer", "ground"},
                                   {"tiles", json::array({1, 1, 1, 1, 1, 0})}},
                              json{{"name", "objects"},
                                   {"render_layer", "low_objects"},
                                   {"tiles", json::array({0, 2, 0, 0, 3, 0})}}})},
                {"exits", json::array()}};
}

// 연습용 타일셋을 읽어 와요 (이것 자체가 실패하면 시험도 실패).
wind::world::Tileset load_tileset() {
    auto result = wind::content::parse_tileset(make_tileset(), "test.json");
    EXPECT_TRUE(result.ok());
    return *result.value;
}

// 오류 목록에 "이 칸 주소"에 대한 오류가 들어 있는지 확인하는 도우미
bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 올바른 타일셋은 잘 읽히고, 벽(2번)은 못 가는 카드로 기억되어야 해요. 없는 9번은 못 찾아야 해요.
TEST(TilesetLoader, ParsesValidTileset) {
    const auto tileset = load_tileset();
    EXPECT_EQ(tileset.id, "test");
    ASSERT_NE(tileset.find(2), nullptr);
    EXPECT_FALSE(tileset.find(2)->walkable);
    EXPECT_EQ(tileset.find(9), nullptr);
}

// 카드 번호가 겹치거나, 그림 위치가 상자 밖("../")을 가리키면 잡아내야 해요.
TEST(TilesetLoader, RejectsDuplicateIdsAndBadPaths) {
    auto doc = make_tileset();
    doc["tiles"].push_back(json{{"id", 1}, {"name", "dup"}, {"walkable", true}});
    doc["image"] = "../outside.bmp";
    const auto result = wind::content::parse_tileset(doc, "test.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "tiles[3].id"));
    EXPECT_TRUE(has_error_at(result.errors, "image"));
}

// 올바른 맵은 잘 읽히고, 칸마다 "지나갈 수 있나?"가 규칙대로 계산되어야 해요.
TEST(MapLoader, ParsesValidMapAndComputesWalkability) {
    const auto tileset = load_tileset();
    const auto result = wind::content::parse_map(make_map(), "unit.json", tileset);
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());

    const auto& map = *result.value;
    EXPECT_EQ(map.width, 3);
    EXPECT_EQ(map.height, 2);
    EXPECT_TRUE(map.is_walkable({0, 0}));
    EXPECT_FALSE(map.is_walkable({1, 0}));   // 벽이 있어서 못 가요
    EXPECT_TRUE(map.is_walkable({1, 1}));    // 풀 위에 나뭇잎: 둘 다 지나갈 수 있어서 OK
    EXPECT_FALSE(map.is_walkable({2, 1}));   // 카드가 하나도 없는 허공이라 못 가요
    EXPECT_FALSE(map.is_walkable({-1, 0}));  // 바둑판 왼쪽 밖
    EXPECT_FALSE(map.is_walkable({3, 0}));   // 바둑판 오른쪽 밖
    ASSERT_NE(map.find_spawn("default"), nullptr);
    EXPECT_EQ(map.find_spawn("default")->position, (TileCoord{0, 0}));
}

// 칸 수가 가로x세로(6칸)와 다르면(3칸) 잡아내야 해요.
TEST(MapLoader, RejectsWrongTileCount) {
    auto doc = make_map();
    doc["layers"][0]["tiles"] = json::array({1, 1, 1});
    const auto result = wind::content::parse_map(doc, "unit.json", load_tileset());
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "layers[0].tiles"));
}

// 타일셋에 없는 카드(99번)를 쓰면, 정확히 그 칸의 주소와 파일 이름을 알려 줘야 해요.
TEST(MapLoader, RejectsUnknownTileIdWithFieldPath) {
    auto doc = make_map();
    doc["layers"][1]["tiles"][2] = 99;
    const auto result = wind::content::parse_map(doc, "unit.json", load_tileset());
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "layers[1].tiles[2]"));
    EXPECT_EQ(result.errors.front().file, "unit.json");
}

// 모르는 양식 버전(2)이면 읽지 않아야 해요.
TEST(MapLoader, RejectsUnsupportedSchemaVersion) {
    auto doc = make_map();
    doc["schema_version"] = 2;
    const auto result = wind::content::parse_map(doc, "unit.json", load_tileset());
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "schema_version"));
}

// 등장 자리가 바둑판 밖이거나 벽 위면 잡아내야 해요.
TEST(MapLoader, RejectsSpawnOutOfBoundsOrBlocked) {
    auto out_of_bounds = make_map();
    out_of_bounds["spawn_points"][0]["x"] = 7;
    EXPECT_TRUE(has_error_at(
        wind::content::parse_map(out_of_bounds, "unit.json", load_tileset()).errors,
        "spawn_points[0].x"));

    auto blocked = make_map();
    blocked["spawn_points"][0]["x"] = 1;  // 벽 위
    EXPECT_TRUE(has_error_at(wind::content::parse_map(blocked, "unit.json", load_tileset()).errors,
                             "spawn_points[0]"));
}

// 모르는 필름 이름("sky")과, 타일셋과 다른 칸 크기(16)를 둘 다 잡아내야 해요.
TEST(MapLoader, RejectsUnknownRenderLayerAndTileSizeMismatch) {
    auto doc = make_map();
    doc["layers"][0]["render_layer"] = "sky";
    doc["tile_size"] = 16;
    const auto result = wind::content::parse_map(doc, "unit.json", load_tileset());
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "layers[0].render_layer"));
    EXPECT_TRUE(has_error_at(result.errors, "tile_size"));
}

// 없는 파일을 읽으라고 하면 멈추지 않고, 어떤 파일이 문제인지 알려 줘야 해요.
TEST(MapLoader, ReportsMissingFile) {
    const auto result = wind::content::load_map_file("does-not-exist", "maps/none.json");
    EXPECT_FALSE(result.ok());
    ASSERT_FALSE(result.errors.empty());
    EXPECT_EQ(result.errors.front().file, "maps/none.json");
}

// 저장소에 들어 있는 진짜 시험 맵도 숙제 검사를 통과해야 해요.
// 누가 맵을 고치다 실수하면 이 시험이 바로 알려 줘요.
TEST(MapLoader, RepositoryTestMapIsValid) {
    const auto result = wind::content::load_map_file(WIND_TEST_CONTENT_DIR, "maps/test_meadow.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_NE(result.value->map.find_spawn("default"), nullptr);
    // 시험 맵에는 꽃잎과 반딧불 분위기 효과가 있어요.
    EXPECT_FALSE(result.value->map.ambience.ambient_light.has_value());  // 초원은 조명 없음
    // 흙길 왼쪽 끝 동굴 입구 (0, 12)
    ASSERT_NE(result.value->map.exit_at({0, 12}), nullptr);
    EXPECT_EQ(result.value->map.exit_at({0, 12})->to_map, "maps/test_cave.json");
    EXPECT_NE(result.value->map.find_spawn("from_cave"), nullptr);
    EXPECT_EQ(result.value->map.ambience.ambient_fx,
              (std::vector<std::string>{"fx/petals.json", "fx/fireflies.json"}));
}

// 분위기(ambience)는 없어도 되고, 있으면 효과 파일 위치를 검사해요.
// 시험 동굴은 조명 맵이에요: 주변광, 주인공 빛, 빛 9개 (횃불 8 + 연못 1)
TEST(MapLoader, RepositoryTestCaveIsValid) {
    const auto result = wind::content::load_map_file(WIND_TEST_CONTENT_DIR, "maps/test_cave.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& ambience = result.value->map.ambience;
    EXPECT_TRUE(ambience.ambient_light.has_value());
    EXPECT_TRUE(ambience.player_light.has_value());
    EXPECT_EQ(ambience.lights.size(), 9u);
    // 왼쪽 벽 출구로 초원에 돌아가요.
    ASSERT_EQ(result.value->map.exits.size(), 1u);
    EXPECT_EQ(result.value->map.exits[0].to_map, "maps/test_meadow.json");
    EXPECT_NE(result.value->map.find_spawn("entrance"), nullptr);
}

// 출구 칸이 전부 벽이면 절대 못 쓰는 출구라서 거절해요. 걸어 들어갈 수 있는 칸이면 통과해요.
// (연습용 맵에서 (1,0)은 벽, (0,1)은 풀)
TEST(MapLoader, RejectsUnreachableExit) {
    auto make_exit = [](int x, int y) {
        return json{{"area", {{"x", x}, {"y", y}, {"w", 1}, {"h", 1}}},
                    {"to_map", "maps/x.json"},
                    {"to_spawn", "a"}};
    };
    auto walled = make_map();
    walled["exits"] = json::array({make_exit(1, 0)});
    const auto bad = wind::content::parse_map(walled, "unit.json", load_tileset());
    ASSERT_FALSE(bad.ok());
    EXPECT_TRUE(has_error_at(bad.errors, "exits[0]"));

    auto open = make_map();
    open["exits"] = json::array({make_exit(0, 1)});
    const auto good = wind::content::parse_map(open, "unit.json", load_tileset());
    ASSERT_TRUE(good.ok());
    EXPECT_NE(good.value->exit_at({0, 1}), nullptr);
}

TEST(MapLoader, ParsesAmbienceAndRejectsBadPaths) {
    const auto tileset = load_tileset();
    EXPECT_TRUE(wind::content::parse_map(make_map(), "unit.json", tileset).value->ambience.ambient_fx.empty());

    auto doc = make_map();
    doc["ambience"] = {{"ambient_fx", {"fx/petals.json", "fx\\rain.json"}}};
    const auto ok = wind::content::parse_map(doc, "unit.json", tileset);
    ASSERT_TRUE(ok.ok());
    EXPECT_EQ(ok.value->ambience.ambient_fx, (std::vector<std::string>{"fx/petals.json", "fx/rain.json"}));

    doc["ambience"] = {{"ambient_fx", {"../escape.json", 7}}};
    const auto bad = wind::content::parse_map(doc, "unit.json", tileset);
    ASSERT_FALSE(bad.ok());
    EXPECT_TRUE(has_error_at(bad.errors, "ambience.ambient_fx[0]"));
    EXPECT_TRUE(has_error_at(bad.errors, "ambience.ambient_fx[1]"));
}

}  // namespace
