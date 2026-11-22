// 이 파일은 무엇을 하나요?
// 필드 화면 준비물 읽기(load_field_assets)를 저장소의 진짜 자료로 숙제 검사해요.
// - 없는 맵 파일이나 없는 등장 자리로 가는 출구는 "못 열어요"(없음)가 나와야 해요.
//   그래야 게임이 지금 맵에 그대로 머물 수 있어요. (명세 7단계 완료 조건)
// - 진짜 맵들(초원, 동굴, 숲)의 출구는 모두 열 수 있는 맵과 등장 자리를 가리켜야 해요.
// - 초원 맵에는 슬라임 두 마리가 있어요 (11a). 맵에 다시 들어오면 쓰러뜨린 슬라임도 다시 나타나요 (11d).
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

// 진짜 초원 맵: 슬라임 두 마리가 맵에 적힌 칸(제자리)에 서 있어요 (11a).
TEST(FieldAssets, MeadowHasSlimesAtHome) {
    const auto assets = load("maps/test_meadow.json", "default");
    ASSERT_TRUE(assets.has_value());
    // 마릿수는 맵 데이터에서 자유롭게 바꾸는 값이라 정확한 수는 보지 않아요 (2 → 40으로 늘렸음)
    ASSERT_FALSE(assets->monsters.empty());
    int slimes = 0;
    for (const auto& monster : assets->monsters) {
        const auto& id = monster.data().id;
        EXPECT_TRUE(id == "slime_green" || id == "frog") << id;  // 13c: 초원에는 개구리도 있어요
        if (id == "slime_green") ++slimes;
        EXPECT_EQ(monster.feet(), monster.home());  // 처음에는 제자리
    }
    EXPECT_GE(slimes, 1);
}

// 맵에 다시 들어오면 슬라임이 다시 나타나요 (11d, 명세 9.5: 13단계 캣닢 쿠션 전까지의 규칙).
// 맵에 들어갈 때마다 준비물을 새로 읽어서, 지난번에 쓰러뜨린 슬라임도 가득 찬 HP로 제자리에 있어요.
TEST(FieldAssets, ReenteringMapBringsSlimesBack) {
    auto first = load("maps/test_meadow.json", "default");
    ASSERT_TRUE(first.has_value());
    ASSERT_FALSE(first->monsters.empty());
    (void)first->monsters[0].take_damage(999);  // 첫 번째 방문에서 쓰러뜨려요
    ASSERT_TRUE(first->monsters[0].defeated());
    const auto again = load("maps/test_meadow.json", "default");  // 맵 이동은 준비물을 새로 읽어요
    ASSERT_TRUE(again.has_value());
    EXPECT_FALSE(again->monsters[0].defeated());
    EXPECT_EQ(again->monsters[0].hp(), again->monsters[0].max_hp());
    EXPECT_EQ(again->monsters[0].feet(), again->monsters[0].home());
    EXPECT_TRUE(again->effects.monster_fx.contains("fx/slime_burst.json"));  // 사라짐 효과도 읽어요
}

}  // namespace
