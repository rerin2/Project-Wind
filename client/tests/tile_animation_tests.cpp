// 이 파일은 무엇을 하나요?
// 움직이는 타일과 흔들리는 타일 계산을 숙제 검사(테스트)해요.
// - 움직이지 않는 카드는 원래 번호 그대로
// - 시간이 지나면 다음 그림으로, 마지막 다음은 처음으로
// - 이웃 칸은 박자가 어긋나요 (모든 물이 한꺼번에 반짝이지 않게)
// - 흔들림은 정수 픽셀이고, 정해진 픽셀보다 더 밀리지 않아요
#include "engine/map/tile_animation.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace {

using wind::engine::animated_tile;
using wind::engine::sway_offset;
using wind::world::TileProperties;

TileProperties water() {
    TileProperties tile{4, "water", false, {}, {}};
    tile.animation = wind::world::TileAnimation{{4, 14, 15}, 0.5f};
    return tile;
}

TEST(TileAnimation, StillTileKeepsItsId) {
    const TileProperties grass{1, "grass", true, {}, {}};
    EXPECT_EQ(animated_tile(grass, 12.3f, {3, 4}), 1);
}

// (0,0) 칸: 0초 -> 4, 0.5초 -> 14, 1초 -> 15, 1.5초 -> 다시 4
TEST(TileAnimation, FramesAdvanceAndLoop) {
    const auto tile = water();
    EXPECT_EQ(animated_tile(tile, 0.0f, {0, 0}), 4);
    EXPECT_EQ(animated_tile(tile, 0.49f, {0, 0}), 4);
    EXPECT_EQ(animated_tile(tile, 0.5f, {0, 0}), 14);
    EXPECT_EQ(animated_tile(tile, 1.0f, {0, 0}), 15);
    EXPECT_EQ(animated_tile(tile, 1.5f, {0, 0}), 4);
}

// 오른쪽 이웃 칸은 같은 시각에 다른 그림이에요. 그래도 같은 칸·같은 시각이면 늘 같은 답.
TEST(TileAnimation, NeighbourCellsAreStaggered) {
    const auto tile = water();
    EXPECT_NE(animated_tile(tile, 0.0f, {0, 0}), animated_tile(tile, 0.0f, {1, 0}));
    EXPECT_EQ(animated_tile(tile, 2.0f, {5, 7}), animated_tile(tile, 2.0f, {5, 7}));
}

// 2픽셀 흔들림: 언제나 -2 ~ 2 사이의 정수이고, 한 주기 동안 양쪽 끝까지 가요.
TEST(TileSway, WholePixelsWithinAmplitude) {
    const wind::world::TileSway sway{2.0f, 4.0f};
    float lowest = 0.0f;
    float highest = 0.0f;
    for (int i = 0; i < 80; ++i) {
        const float offset = sway_offset(sway, static_cast<float>(i) * 0.05f, {0, 0});
        EXPECT_EQ(offset, std::round(offset));
        EXPECT_LE(std::abs(offset), 2.0f);
        lowest = std::min(lowest, offset);
        highest = std::max(highest, offset);
    }
    EXPECT_EQ(lowest, -2.0f);
    EXPECT_EQ(highest, 2.0f);
}

// 바람이 물결처럼 지나가요: 같은 시각에 멀리 떨어진 칸은 다르게 밀려 있어요.
// 흔들림 0픽셀이면 전혀 안 움직여요.
TEST(TileSway, WaveMovesAcrossColumnsAndZeroIsStill) {
    const wind::world::TileSway sway{2.0f, 4.0f};
    EXPECT_NE(sway_offset(sway, 1.0f, {0, 0}), sway_offset(sway, 1.0f, {3, 0}));
    EXPECT_EQ(sway_offset({0.0f, 4.0f}, 1.0f, {0, 0}), 0.0f);
}

}  // namespace
