// 이 파일은 무엇을 하나요?
// 캐릭터를 한 칸씩 옮기는 장치(GridMover)를 숙제 검사(테스트)해요.
// - 한 칸 잘 가나, 가는 동안 스르륵 중간 위치가 나오나
// - 벽이나 맵 끝으로는 못 가나 (고개만 돌리기)
// - 걷는 중에 또 부탁하면 기다리라고 하나
// - 도착할 때 남은 시간을 돌려주나 (다음 칸으로 끊김 없이 가려고)
#include "game/entities/grid_mover.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::GridMover;
using wind::world::Direction;
using wind::world::TileCoord;

// 검사용 3x3 맵. 1 = 갈 수 있음, 0 = 막힘. 가운데 줄 오른쪽 칸 (2,1)만 막혀 있어요.
wind::world::MapData make_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 3;
    map.height = 3;
    map.walkable = {1, 1, 1,
                    1, 1, 0,
                    1, 1, 1};
    return map;
}

constexpr double kStep = 0.2;  // 한 칸 가는 데 0.2초

// 아래로 한 칸 가기: 게임 규칙상 위치는 바로 도착 칸이 되고,
// 화면 위치는 반쯤 갔을 때 1.5, 다 가면 2.0이어야 해요.
TEST(GridMover, MovesOneTileAndArrives) {
    const auto map = make_map();
    GridMover mover({1, 1}, Direction::Down, kStep);
    EXPECT_EQ(mover.try_move(Direction::Down, map), GridMover::MoveResult::Started);
    // 규칙상 위치는 바로 도착 칸, 화면 위치는 스르륵 미끄러지는 중간 위치
    EXPECT_EQ(mover.tile(), (TileCoord{1, 2}));
    mover.update(kStep / 2);
    EXPECT_TRUE(mover.moving());
    EXPECT_NEAR(mover.visual_tile().y, 1.5f, 1e-5f);
    mover.update(kStep / 2);
    EXPECT_FALSE(mover.moving());
    EXPECT_EQ(mover.visual_tile(), (wind::engine::Vec2{1.0f, 2.0f}));
}

// 막힌 칸 쪽으로 가라고 하면 움직이지 않지만, 그쪽으로 고개는 돌려야 해요.
TEST(GridMover, BlockedTileOnlyTurns) {
    const auto map = make_map();
    GridMover mover({1, 1}, Direction::Down, kStep);
    EXPECT_EQ(mover.try_move(Direction::Right, map), GridMover::MoveResult::Blocked);
    EXPECT_EQ(mover.tile(), (TileCoord{1, 1}));
    EXPECT_EQ(mover.facing(), Direction::Right);
    EXPECT_FALSE(mover.moving());
}

// 왼쪽 위 구석(0,0)에서 위나 왼쪽으로 가면 맵 밖이라 못 가야 해요.
TEST(GridMover, CannotLeaveMap) {
    const auto map = make_map();
    GridMover mover({0, 0}, Direction::Down, kStep);
    EXPECT_EQ(mover.try_move(Direction::Up, map), GridMover::MoveResult::Blocked);
    EXPECT_EQ(mover.try_move(Direction::Left, map), GridMover::MoveResult::Blocked);
    EXPECT_EQ(mover.tile(), (TileCoord{0, 0}));
}

// 한 칸 가는 도중에 또 가라고 하면 "아직 가는 중"(Busy)이라고 해야 해요.
// 한 번에 두 칸을 건너뛰면 안 되니까요.
TEST(GridMover, RejectsMoveWhileMoving) {
    const auto map = make_map();
    GridMover mover({0, 0}, Direction::Down, kStep);
    ASSERT_EQ(mover.try_move(Direction::Down, map), GridMover::MoveResult::Started);
    EXPECT_EQ(mover.try_move(Direction::Down, map), GridMover::MoveResult::Busy);
    EXPECT_EQ(mover.tile(), (TileCoord{0, 1}));
    EXPECT_EQ(mover.facing(), Direction::Down);
}

// 0.2초짜리 이동에 0.1초 + 0.15초 = 0.25초가 흐르면, 도착하고 0.05초가 남아야 해요.
// 이 남은 시간을 다음 칸 이동에 이어 써서 걸음이 덜컹거리지 않아요.
TEST(GridMover, ReturnsLeftoverTimeOnArrival) {
    const auto map = make_map();
    GridMover mover({0, 0}, Direction::Down, kStep);
    mover.try_move(Direction::Down, map);
    EXPECT_DOUBLE_EQ(mover.update(0.1), 0.0);
    EXPECT_NEAR(mover.update(0.15), 0.05, 1e-9);
}

// 걷는 중에 순간이동하면 걷기가 멈추고 새 칸에 바로 서 있어야 해요.
TEST(GridMover, TeleportStopsMovement) {
    const auto map = make_map();
    GridMover mover({0, 0}, Direction::Down, kStep);
    mover.try_move(Direction::Down, map);
    mover.teleport({2, 2});
    EXPECT_FALSE(mover.moving());
    EXPECT_EQ(mover.tile(), (TileCoord{2, 2}));
}

}  // namespace
