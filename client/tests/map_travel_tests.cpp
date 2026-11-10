// 이 파일은 무엇을 하나요?
// 맵 옮기기 담당(MapTravel)을 숙제 검사(테스트)해요.
// - 출구를 밟으면 화면이 다 가려진 순간 딱 한 번 새 맵을 열어요
// - 잘못된 출구(새 맵을 못 엶)면 지금 맵에 그대로 있고, 오류 글자를 잠깐 보여 줘요
// - 옮기는 중에는 다른 출구를 받지 않아요
// - 출구에 적힌 모양(아이리스)으로 가려요
#include "game/world/map_travel.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using wind::engine::ScreenFade;
using wind::game::MapTravel;

wind::world::MapExit make_exit(std::string to_map) {
    wind::world::MapExit exit;
    exit.to_map = std::move(to_map);
    exit.to_spawn = "entrance";
    return exit;
}

// 시험용 "게임": 지금 맵 이름과, 맵을 연 횟수를 기억해요.
// 이름이 "maps/missing.json"인 맵은 없어서 못 열어요.
struct FakeWorld {
    std::string current = "maps/meadow.json";
    int attempts = 0;
    bool enter(const wind::world::MapExit& exit) {
        ++attempts;
        if (exit.to_map == "maps/missing.json") {
            return false;  // 못 열었으니 current는 그대로
        }
        current = exit.to_map;
        return true;
    }
};

// 0.1초씩 n번 진행
void run(MapTravel& travel, FakeWorld& world, int steps) {
    for (int i = 0; i < steps; ++i) {
        travel.update(0.1f, [&](const wind::world::MapExit& e) { return world.enter(e); });
    }
}

TEST(MapTravel, EntersNewMapOnceWhenCovered) {
    MapTravel travel(0.3f);
    FakeWorld world;
    ASSERT_TRUE(travel.begin(make_exit("maps/forest.json")));
    EXPECT_TRUE(travel.busy());
    run(travel, world, 2);  // 0.2초: 아직 다 안 가려졌어요
    EXPECT_EQ(world.attempts, 0);
    run(travel, world, 1);  // 0.3초: 다 가려진 순간 열어요
    EXPECT_EQ(world.attempts, 1);
    EXPECT_EQ(world.current, "maps/forest.json");
    run(travel, world, 5);  // 다시 밝아지고 끝
    EXPECT_EQ(world.attempts, 1);
    EXPECT_FALSE(travel.busy());
    EXPECT_TRUE(travel.notice().empty());
}

// 명세 7단계 완료 조건: 잘못된 출구 대상이면 이전 맵에 머물고 오류를 보여 줘요.
TEST(MapTravel, BadTargetStaysOnCurrentMapAndShowsNotice) {
    MapTravel travel(0.3f);
    FakeWorld world;
    ASSERT_TRUE(travel.begin(make_exit("maps/missing.json")));
    run(travel, world, 3);
    EXPECT_EQ(world.attempts, 1);
    EXPECT_EQ(world.current, "maps/meadow.json");
    EXPECT_NE(travel.notice().find("maps/missing.json"), std::string::npos);
    // 화면은 다시 밝아져서 계속 놀 수 있어요.
    run(travel, world, 5);
    EXPECT_FALSE(travel.busy());
    EXPECT_EQ(travel.fade().phase(), ScreenFade::Phase::Clear);
    // 오류 글자는 4초쯤 뒤에 사라져요.
    EXPECT_FALSE(travel.notice().empty());
    run(travel, world, 40);
    EXPECT_TRUE(travel.notice().empty());
}

TEST(MapTravel, IgnoresSecondExitWhileBusy) {
    MapTravel travel(0.3f);
    FakeWorld world;
    ASSERT_TRUE(travel.begin(make_exit("maps/forest.json")));
    EXPECT_FALSE(travel.begin(make_exit("maps/cave.json")));
    run(travel, world, 10);
    EXPECT_EQ(world.current, "maps/forest.json");
    EXPECT_EQ(world.attempts, 1);
}

TEST(MapTravel, UsesTheExitTransitionStyle) {
    MapTravel travel(0.3f);
    auto exit = make_exit("maps/cave.json");
    exit.transition = wind::world::ScreenTransition::Iris;
    ASSERT_TRUE(travel.begin(exit));
    EXPECT_EQ(travel.fade().style(), wind::world::ScreenTransition::Iris);
}

}  // namespace
