// 이 파일은 무엇을 하나요?
// 맵 출구(exits) 읽기와 "이 칸에 출구가 있나?" 찾기를 숙제 검사(테스트)해요.
#include "wind/content/exit_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::ErrorList;
using wind::content::parse_exits;

bool has_error_at(const ErrorList& errors, std::string_view field) {
    return std::ranges::any_of(errors.items(), [field](const auto& e) { return e.field == field; });
}

wind::world::MapData make_map() {
    wind::world::MapData map;
    map.width = 10;
    map.height = 8;
    return map;
}

json make_exit() {
    return json{{"area", {{"x", 0}, {"y", 3}, {"w", 1}, {"h", 2}}},
                {"to_map", "maps\\cave.json"},
                {"to_spawn", "entrance"}};
}

// 올바른 출구: 영역은 (0,3)~(0,4) 두 칸, 파일 위치는 "/"로 정리돼요.
TEST(ExitLoader, ParsesExitAndFindsIt) {
    auto map = make_map();
    ErrorList errors;
    parse_exits(json::array({make_exit()}), "meadow.json", map, errors);
    ASSERT_TRUE(errors.empty()) << wind::content::to_string(errors.items().front());
    ASSERT_EQ(map.exits.size(), 1u);
    EXPECT_EQ(map.exits[0].to_map, "maps/cave.json");
    EXPECT_EQ(map.exits[0].to_spawn, "entrance");
    // 출구 영역 안 칸에서는 찾고, 밖에서는 못 찾아요.
    EXPECT_NE(map.exit_at({0, 3}), nullptr);
    EXPECT_NE(map.exit_at({0, 4}), nullptr);
    EXPECT_EQ(map.exit_at({0, 5}), nullptr);
    EXPECT_EQ(map.exit_at({1, 3}), nullptr);
}

// 맵 밖으로 나가는 영역, 폴더 밖 맵 파일, 빠진 등장 자리는 거절해요.
TEST(ExitLoader, ReportsBadExits) {
    auto map = make_map();
    auto outside = make_exit();
    outside["area"]["x"] = 9;
    outside["area"]["w"] = 3;
    auto escape = make_exit();
    escape["to_map"] = "../secret.json";
    auto no_spawn = make_exit();
    no_spawn.erase("to_spawn");
    ErrorList errors;
    parse_exits(json::array({outside, escape, no_spawn}), "meadow.json", map, errors);
    EXPECT_TRUE(has_error_at(errors, "exits[0].area"));
    EXPECT_TRUE(has_error_at(errors, "exits[1].to_map"));
    EXPECT_TRUE(has_error_at(errors, "exits[2].to_spawn"));
    EXPECT_TRUE(map.exits.empty());
}

}  // namespace

namespace {

// 화면 가리는 방법: 없으면 페이드, "iris"면 아이리스, 모르는 이름은 거절해요.
TEST(ExitLoader, ParsesTransition) {
    auto map = make_map();
    auto iris = make_exit();
    iris["transition"] = "iris";
    auto bad = make_exit();
    bad["transition"] = "spin";
    ErrorList errors;
    parse_exits(json::array({make_exit(), iris, bad}), "meadow.json", map, errors);
    EXPECT_TRUE(has_error_at(errors, "exits[2].transition"));
    ASSERT_EQ(map.exits.size(), 2u);
    EXPECT_EQ(map.exits[0].transition, wind::world::ScreenTransition::Fade);
    EXPECT_EQ(map.exits[1].transition, wind::world::ScreenTransition::Iris);
}

}  // namespace
