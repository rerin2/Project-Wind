// 이 파일은 무엇을 하나요?
// 출구 지켜보기 검사 (7단계 규칙을 옮긴 것, 13b): 무장 뒤에만 동작, 한 번만 꺼냄, 쓰러짐·쉬기 중에는 안 떠남.
#include "game/world/exit_watcher.hpp"

#include <gtest/gtest.h>

using wind::engine::Vec2;
using wind::game::ExitWatcher;

namespace {

// 5x1 맵, 32픽셀 칸, 맨 오른쪽 칸(4,0)이 출구
wind::world::MapData exit_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 5;
    map.height = 1;
    map.walkable.assign(5, 1);
    wind::world::MapExit exit;
    exit.area_min = {4, 0};
    exit.area_max = {4, 0};
    exit.to_map = "maps/next.json";
    map.exits.push_back(exit);
    return map;
}

constexpr Vec2 kOnExit{144.0f, 16.0f};   // 칸 (4,0)
constexpr Vec2 kOffExit{48.0f, 16.0f};   // 칸 (1,0)

}  // namespace

TEST(ExitWatcher, NeedsToBeArmedFirst) {
    const auto map = exit_map();
    ExitWatcher watcher;
    watcher.step(map, kOnExit, true);  // 출구 위에서 나타났어요: 아직 무장 전
    EXPECT_FALSE(watcher.take_request().has_value());
    watcher.step(map, kOffExit, true);  // 출구 밖을 밟고
    watcher.step(map, kOnExit, true);   // 다시 출구로
    const auto request = watcher.take_request();
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->to_map, "maps/next.json");
    EXPECT_FALSE(watcher.take_request().has_value());  // 한 번만
}

TEST(ExitWatcher, DoesNotLeaveWhenNotAllowed) {
    const auto map = exit_map();
    ExitWatcher watcher;
    watcher.step(map, kOffExit, true);
    watcher.step(map, kOnExit, false);  // 쓰러졌거나 쉬는 중
    EXPECT_FALSE(watcher.take_request().has_value());
}
