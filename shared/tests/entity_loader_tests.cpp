// 이 파일은 무엇을 하나요?
// 맵 물건(entities) 읽기를 숙제 검사(테스트)해요.
// - 올바른 허수아비 한 개를 읽어요
// - 모르는 종류, 맵 밖, id 중복, 같은 칸, 폴더 밖 파일, 등장 자리·출구 위는 거절해요
#include "wind/content/entity_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::ErrorList;
using wind::content::parse_entities;

bool has_error_at(const ErrorList& errors, std::string_view field) {
    return std::ranges::any_of(errors.items(), [field](const auto& e) { return e.field == field; });
}

// 10x8 맵, 등장 자리 (1,1), 출구 (0,4)
wind::world::MapData make_map() {
    wind::world::MapData map;
    map.width = 10;
    map.height = 8;
    map.spawn_points.push_back({"default", {1, 1}});
    wind::world::MapExit exit;
    exit.area_min = {0, 4};
    exit.area_max = {0, 4};
    map.exits.push_back(exit);
    return map;
}

json dummy(std::string id, int x, int y) {
    return json{{"type", "training_dummy"}, {"id", id}, {"x", x}, {"y", y},
                {"data", "props\\training_dummy.json"}};
}

TEST(EntityLoader, ParsesTrainingDummy) {
    auto map = make_map();
    ErrorList errors;
    parse_entities(json::array({dummy("d1", 5, 3)}), "meadow.json", map, errors);
    ASSERT_TRUE(errors.empty()) << wind::content::to_string(errors.items().front());
    ASSERT_EQ(map.entities.size(), 1u);
    EXPECT_EQ(map.entities[0].type, wind::world::EntityType::TrainingDummy);
    EXPECT_EQ(map.entities[0].id, "d1");
    EXPECT_EQ(map.entities[0].tile, (wind::world::TileCoord{5, 3}));
    EXPECT_EQ(map.entities[0].data, "props/training_dummy.json");  // "\"는 "/"로 정리돼요
}

TEST(EntityLoader, RejectsBadEntities) {
    auto map = make_map();
    auto unknown = dummy("u", 2, 2);
    unknown["type"] = "dragon";             // 모르는 종류
    auto outside = dummy("o", 10, 2);       // 가로 10칸 맵에서 x = 10은 밖
    auto escape = dummy("e", 3, 3);
    escape["data"] = "../secret.json";      // 콘텐츠 폴더 밖
    ErrorList errors;
    parse_entities(json::array({unknown, outside, escape}), "m.json", map, errors);
    EXPECT_TRUE(has_error_at(errors, "entities[0].type"));
    EXPECT_TRUE(has_error_at(errors, "entities[1].x"));
    EXPECT_TRUE(has_error_at(errors, "entities[2].data"));
    EXPECT_TRUE(map.entities.empty());
}

TEST(EntityLoader, RejectsOverlaps) {
    auto map = make_map();
    ErrorList errors;
    parse_entities(json::array({dummy("a", 5, 3), dummy("a", 6, 3),   // id 중복
                                dummy("b", 5, 3),                       // 같은 칸
                                dummy("c", 1, 1),                       // 등장 자리 위
                                dummy("d", 0, 4)}),                     // 출구 위
                   "m.json", map, errors);
    EXPECT_TRUE(has_error_at(errors, "entities[1].id"));
    EXPECT_TRUE(has_error_at(errors, "entities[2]"));
    EXPECT_TRUE(has_error_at(errors, "entities[3]"));
    EXPECT_TRUE(has_error_at(errors, "entities[4]"));
    ASSERT_EQ(map.entities.size(), 1u);  // 처음 것만 남아요
}

TEST(EntityLoader, RejectsNonArray) {
    auto map = make_map();
    ErrorList errors;
    parse_entities(json::object(), "m.json", map, errors);
    EXPECT_TRUE(has_error_at(errors, "entities"));
}

}  // namespace
