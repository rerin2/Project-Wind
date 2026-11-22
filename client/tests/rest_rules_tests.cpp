// 이 파일은 무엇을 하나요?
// 쉬기 조건 검사 (13b, 명세 8.7): 24픽셀 경계, 가장 가까운 쿠션, 몬스터가 노리거나 3초 안에 맞았으면 못 쉼.
#include "game/world/rest_rules.hpp"

#include <gtest/gtest.h>

using wind::game::Cushion;
using wind::game::RestCheck;

namespace {

std::vector<Cushion> two_cushions() {
    return {Cushion{"near", "가까운", {3, 3}, {112.0f, 128.0f}}, Cushion{"far", "먼", {6, 3}, {208.0f, 128.0f}}};
}

}  // namespace

TEST(RestRules, ReachIs24Pixels) {
    const auto cushions = two_cushions();
    EXPECT_TRUE(wind::game::nearest_cushion({112.0f + 24.0f, 128.0f}, cushions).has_value());   // 딱 24는 돼요
    EXPECT_FALSE(wind::game::nearest_cushion({112.0f + 24.5f, 128.0f}, cushions).has_value());  // 넘으면 안 돼요
}

TEST(RestRules, PicksNearestCushion) {
    const auto cushions = two_cushions();
    EXPECT_EQ(wind::game::nearest_cushion({200.0f, 128.0f}, cushions), 1u);
    EXPECT_EQ(wind::game::nearest_cushion({110.0f, 130.0f}, cushions), 0u);
}

TEST(RestRules, CannotRestWhileMonsterEngagedOrJustHit) {
    const auto cushions = two_cushions();
    const wind::engine::Vec2 feet{112.0f, 128.0f};
    EXPECT_EQ(wind::game::check_rest(feet, cushions, true, 100.0f).result, RestCheck::TooNoisy);   // 쫓기는 중
    EXPECT_EQ(wind::game::check_rest(feet, cushions, false, 2.9f).result, RestCheck::TooNoisy);    // 맞은 지 2.9초
    EXPECT_EQ(wind::game::check_rest(feet, cushions, false, 3.0f).result, RestCheck::Ok);          // 3초면 돼요
    EXPECT_EQ(wind::game::check_rest({0.0f, 0.0f}, cushions, true, 0.0f).result, RestCheck::NoCushion);  // 멀면 그냥 없음
}
