// 이 파일은 무엇을 하나요?
// "칸 번호 <-> 픽셀 위치" 계산기가 맞게 계산하는지 숙제 검사(테스트)를 해요. (명세 5.3)
// EXPECT_EQ(A, B)는 "A와 B가 같아야 해요"라는 뜻이에요. 다르면 검사에 떨어져요.
#include "engine/map/coordinates.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Vec2;
using wind::world::TileCoord;

// 칸 번호 x 32 = 그 칸 왼쪽 위 모서리의 픽셀 위치여야 해요. 맵 밖(-1번 칸)도 마찬가지.
TEST(Coordinates, TileToWorldIsTopLeftCorner) {
    EXPECT_EQ(wind::engine::tile_to_world({0, 0}, 32), (Vec2{0.0f, 0.0f}));
    EXPECT_EQ(wind::engine::tile_to_world({3, 2}, 32), (Vec2{96.0f, 64.0f}));
    EXPECT_EQ(wind::engine::tile_to_world({-1, 0}, 32), (Vec2{-32.0f, 0.0f}));
}

// 칸의 한가운데는 왼쪽 위에서 반 칸(16픽셀)씩 들어간 곳이에요.
TEST(Coordinates, TileCenterIsHalfTileIn) {
    EXPECT_EQ(wind::engine::tile_center({1, 1}, 32), (Vec2{48.0f, 48.0f}));
}

// 한 칸 안의 픽셀은 모두 같은 칸 번호가 나와야 해요. 딱 경계선(32픽셀)은 다음 칸이에요.
// 맵 왼쪽 바깥(음수)도 내림을 해서 -1, -2번 칸이 맞게 나와야 해요.
TEST(Coordinates, WorldToTileFloorsIncludingNegatives) {
    EXPECT_EQ(wind::engine::world_to_tile({0.0f, 0.0f}, 32), (TileCoord{0, 0}));
    EXPECT_EQ(wind::engine::world_to_tile({31.9f, 31.9f}, 32), (TileCoord{0, 0}));
    EXPECT_EQ(wind::engine::world_to_tile({32.0f, 64.0f}, 32), (TileCoord{1, 2}));
    EXPECT_EQ(wind::engine::world_to_tile({-0.5f, -32.0f}, 32), (TileCoord{-1, -1}));
    EXPECT_EQ(wind::engine::world_to_tile({-33.0f, 0.0f}, 32), (TileCoord{-2, 0}));
}

// 칸 -> 픽셀(한가운데) -> 다시 칸으로 바꾸면 처음 칸으로 돌아와야 해요. (왕복 검사)
// 칸 크기가 16, 24, 48이어도 똑같이 맞아야 해요.
TEST(Coordinates, RoundTripForOtherTileSizes) {
    for (int size : {16, 24, 32, 48}) {
        for (TileCoord tile : {TileCoord{0, 0}, TileCoord{5, 9}, TileCoord{-3, 4}}) {
            EXPECT_EQ(wind::engine::world_to_tile(wind::engine::tile_center(tile, size), size), tile)
                << "tile size " << size;
        }
    }
}

}  // namespace
