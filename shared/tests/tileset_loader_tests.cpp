// 이 파일은 무엇을 하나요?
// 타일셋 설명서 숙제 검사(tileset_loader)를 시험해요.
// - 올바른 카드 목록 읽기, 번호 중복·나쁜 그림 위치 잡기
// - 움직이는 카드(animation)와 흔들리는 카드(sway) 읽기
// - 없는 그림 번호, 그림 1장짜리 움직임, 범위 밖 숫자 잡기
#include "wind/content/tileset_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>
#include <vector>

namespace {

using nlohmann::json;

// 연습용 타일셋: 1번 = 풀, 2번 = 벽(못 감), 3번 = 나뭇잎, 4번 = 물, 5번 = 물 반짝임 그림
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
                              json{{"id", 3}, {"name", "canopy"}, {"walkable", true}},
                              json{{"id", 4}, {"name", "water"}, {"walkable", false}},
                              json{{"id", 5}, {"name", "water_shine"}, {"walkable", false}}})}};
}

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 올바른 타일셋은 잘 읽히고, 벽(2번)은 못 가는 카드로 기억되어야 해요. 없는 9번은 못 찾아야 해요.
TEST(TilesetLoader, ParsesValidTileset) {
    const auto result = wind::content::parse_tileset(make_tileset(), "test.json");
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value->id, "test");
    ASSERT_NE(result.value->find(2), nullptr);
    EXPECT_FALSE(result.value->find(2)->walkable);
    EXPECT_FALSE(result.value->find(2)->animation.has_value());
    EXPECT_EQ(result.value->find(9), nullptr);
}

// 카드 번호가 겹치거나, 그림 위치가 상자 밖("../")을 가리키면 잡아내야 해요.
TEST(TilesetLoader, RejectsDuplicateIdsAndBadPaths) {
    auto doc = make_tileset();
    doc["tiles"].push_back(json{{"id", 1}, {"name", "dup"}, {"walkable", true}});
    doc["image"] = "../outside.bmp";
    const auto result = wind::content::parse_tileset(doc, "test.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "tiles[5].id"));
    EXPECT_TRUE(has_error_at(result.errors, "image"));
}

// 물(4번)은 4 -> 5 -> 4 그림을 0.3초씩, 나뭇잎(3번)은 1픽셀씩 3초 주기로 흔들려요.
TEST(TilesetLoader, ParsesAnimationAndSway) {
    auto doc = make_tileset();
    doc["tiles"][3]["animation"] = {{"frames", {4, 5, 4}}, {"frame_seconds", 0.3}};
    doc["tiles"][2]["sway"] = {{"pixels", 1}, {"period_seconds", 3}};
    const auto result = wind::content::parse_tileset(doc, "test.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());

    const auto* water = result.value->find(4);
    ASSERT_TRUE(water->animation.has_value());
    EXPECT_EQ(water->animation->frames, (std::vector<wind::world::TileId>{4, 5, 4}));
    EXPECT_FLOAT_EQ(water->animation->frame_seconds, 0.3f);
    const auto* canopy = result.value->find(3);
    ASSERT_TRUE(canopy->sway.has_value());
    EXPECT_FLOAT_EQ(canopy->sway->pixels, 1.0f);
    EXPECT_FLOAT_EQ(canopy->sway->period_seconds, 3.0f);
}

// 타일셋에 없는 그림 번호(9), 그림 1장, 0초짜리 그림, 너무 큰 흔들림은 모두 틀린 것이에요.
TEST(TilesetLoader, RejectsBadAnimationAndSway) {
    auto doc = make_tileset();
    doc["tiles"][3]["animation"] = {{"frames", {4, 9}}, {"frame_seconds", 0.3}};
    doc["tiles"][4]["animation"] = {{"frames", {5}}, {"frame_seconds", 0.3}};
    doc["tiles"][0]["animation"] = {{"frames", {1, 1}}, {"frame_seconds", 0}};
    doc["tiles"][2]["sway"] = {{"pixels", 20}, {"period_seconds", 3}};
    const auto result = wind::content::parse_tileset(doc, "test.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "tiles[3].animation.frames[1]"));
    EXPECT_TRUE(has_error_at(result.errors, "tiles[4].animation.frames"));
    EXPECT_TRUE(has_error_at(result.errors, "tiles[0].animation.frame_seconds"));
    EXPECT_TRUE(has_error_at(result.errors, "tiles[2].sway.pixels"));
}

}  // namespace
