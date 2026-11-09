// 이 파일은 무엇을 하나요?
// 화살표를 4방향 그림으로 고르는 규칙을 숙제 검사(테스트)해요.
#include "game/entities/facing.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::facing_from_vector;
using wind::world::Direction;

// 더 많이 가는 축을 따라요.
TEST(Facing, DominantAxisWins) {
    EXPECT_EQ(facing_from_vector({1.0f, 0.2f}, Direction::Up), Direction::Right);
    EXPECT_EQ(facing_from_vector({-0.1f, -1.0f}, Direction::Down), Direction::Up);
}

// 정확히 대각선이면 지금 보는 방향이 둘 중 하나일 때 그대로 둬요. (깜빡임 방지)
TEST(Facing, ExactDiagonalKeepsCurrent) {
    EXPECT_EQ(facing_from_vector({1.0f, -1.0f}, Direction::Up), Direction::Up);
    EXPECT_EQ(facing_from_vector({1.0f, -1.0f}, Direction::Right), Direction::Right);
    EXPECT_EQ(facing_from_vector({1.0f, -1.0f}, Direction::Down), Direction::Right);
}

// 화살표가 (0, 0)이면 방향을 바꾸지 않아요.
TEST(Facing, ZeroVectorKeepsCurrent) {
    EXPECT_EQ(facing_from_vector({0.0f, 0.0f}, Direction::Left), Direction::Left);
}

}  // namespace
