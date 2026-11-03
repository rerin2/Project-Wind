// 이 파일은 무엇을 하나요?
// 방향 코드가 제대로 동작하는지 자동으로 확인하는 시험지(테스트)예요.
// TEST(...) 하나가 문제 하나이고, EXPECT_...는 "이 답이 나와야 해요"라는 채점 기준이에요.
#include "wind/world/direction.hpp"

#include <gtest/gtest.h>

namespace {

using wind::world::Direction;
using wind::world::TileCoord;

// (5,5) 칸에서 네 방향으로 한 칸씩 가 봐요.
// y는 아래로 갈수록 커지니까 위(Up)는 y-1, 아래(Down)는 y+1이어야 해요.
TEST(Direction, StepMovesOneTile) {
    const TileCoord origin{5, 5};
    EXPECT_EQ(wind::world::step(origin, Direction::Up), (TileCoord{5, 4}));
    EXPECT_EQ(wind::world::step(origin, Direction::Down), (TileCoord{5, 6}));
    EXPECT_EQ(wind::world::step(origin, Direction::Left), (TileCoord{4, 5}));
    EXPECT_EQ(wind::world::step(origin, Direction::Right), (TileCoord{6, 5}));
}

// 방향 -> 글자 -> 방향으로 바꾸면 원래 방향이 돌아와야 해요.
// "north"나 대문자 "Down"처럼 약속에 없는 글자는 거절해야 해요.
TEST(Direction, NamesRoundTrip) {
    for (Direction dir : wind::world::kAllDirections) {
        EXPECT_EQ(wind::world::parse_direction(wind::world::to_string(dir)), dir);
    }
    EXPECT_FALSE(wind::world::parse_direction("north").has_value());
    EXPECT_FALSE(wind::world::parse_direction("Down").has_value());
}

}  // namespace
