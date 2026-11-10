// 이 파일은 무엇을 하나요?
// 필드 화면 준비물 읽기(load_field_assets)를 저장소의 진짜 자료로 숙제 검사해요.
// - 없는 맵 파일이나 없는 등장 자리로 가는 출구는 "못 열어요"(없음)가 나와야 해요.
//   그래야 게임이 지금 맵에 그대로 머물 수 있어요. (명세 7단계 완료 조건)
// - 진짜 맵들(초원, 동굴, 숲)의 출구는 모두 열 수 있는 맵과 등장 자리를 가리켜야 해요.
#include "game/states/field_assets.hpp"

#include <gtest/gtest.h>

#include <set>
#include <string>
#include <vector>

namespace {

constexpr std::string_view kSheet = "sprites/test_hero.json";

std::optional<wind::game::FieldAssets> load(std::string_view map, std::string_view spawn) {
    return wind::game::load_field_assets(WIND_TEST_CONTENT_DIR, map, kSheet, spawn, 0);
}

TEST(FieldAssets, MissingMapFileFails) {
    EXPECT_FALSE(load("maps/does_not_exist.json", "default").has_value());
}

TEST(FieldAssets, MissingSpawnPointFails) {
    EXPECT_FALSE(load("maps/test_meadow.json", "no_such_spawn").has_value());
}

TEST(FieldAssets, PathOutsideContentFails) {
    EXPECT_FALSE(load("../CMakeLists.txt", "default").has_value());
}

// 맵에서 맵으로 출구를 따라가며, 갈 수 있는 맵을 모두 열어 봐요.
TEST(FieldAssets, EveryContentExitLeadsSomewhereValid) {
    std::vector<std::string> todo{"maps/test_meadow.json"};
    std::set<std::string> seen{todo.front()};
    while (!todo.empty()) {
        const std::string map = todo.back();
        todo.pop_back();
        const auto assets = load(map, "default");
        ASSERT_TRUE(assets.has_value()) << map;
        for (const auto& exit : assets->map.map.exits) {
            EXPECT_TRUE(load(exit.to_map, exit.to_spawn).has_value())
                << map << " -> " << exit.to_map << " spawn " << exit.to_spawn;
            if (seen.insert(exit.to_map).second) {
                todo.push_back(exit.to_map);
            }
        }
    }
    // 초원에서 동굴과 숲까지 갈 수 있어요.
    EXPECT_TRUE(seen.contains("maps/test_cave.json"));
    EXPECT_TRUE(seen.contains("maps/test_forest.json"));
}

}  // namespace
