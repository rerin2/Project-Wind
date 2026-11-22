// 이 파일은 무엇을 하나요?
// 옮길 곳 정하기 검사 (13b): 쉰 적이 없으면 지금 맵 기본 자리, 쉬었으면 마지막 쿠션에서 부활.
#include "game/world/travel_targets.hpp"

#include <gtest/gtest.h>

using wind::game::HeroProgress;

TEST(TravelTargets, RespawnAtMapDefaultBeforeAnyRest) {
    const HeroProgress progress(wind::progression::ProgressionRules{});
    const auto target = wind::game::respawn_target(progress, "maps/test_forest.json");
    EXPECT_EQ(target.to_map, "maps/test_forest.json");
    EXPECT_EQ(target.to_spawn, "default");
    EXPECT_EQ(target.transition, wind::world::ScreenTransition::Fade);
}

TEST(TravelTargets, RespawnAtLastCushionEvenFromAnotherMap) {
    HeroProgress progress(wind::progression::ProgressionRules{});
    (void)progress.rest_at("maps/test_meadow.json", "meadow_cushion");
    const auto target = wind::game::respawn_target(progress, "maps/test_forest.json");  // 숲에서 쓰러져도
    EXPECT_EQ(target.to_map, "maps/test_meadow.json");                                  // 초원 쿠션에서 깨어나요
    EXPECT_EQ(target.to_spawn, "meadow_cushion");
}

TEST(TravelTargets, CushionTargetIsSameMapCushion) {
    const auto target = wind::game::cushion_target("maps/test_meadow.json", "meadow_cushion");
    EXPECT_EQ(target.to_map, "maps/test_meadow.json");
    EXPECT_EQ(target.to_spawn, "meadow_cushion");
}
