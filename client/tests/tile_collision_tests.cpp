// 이 파일은 무엇을 하나요?
// 발 상자가 벽에 막히고, 벽을 따라 미끄러지고, 모서리를 봐주는지 숙제 검사(테스트)를 해요.
#include "game/physics/tile_collision.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Vec2;
using wind::game::BodyShape;
using wind::game::move_and_slide;

// 검사용 5x5 맵, 칸 크기 32. 1 = 갈 수 있음, 0 = 벽.
// 가운데 줄 (2,2)에만 벽이 있어요.
wind::world::MapData make_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 5;
    map.height = 5;
    map.walkable = {1, 1, 1, 1, 1,
                    1, 1, 1, 1, 1,
                    1, 1, 0, 1, 1,
                    1, 1, 1, 1, 1,
                    1, 1, 1, 1, 1};
    return map;
}

const BodyShape kBody{14.0f, 8.0f};

// 벽 (2,2)는 x 64~96이에요. 왼쪽에서 오른쪽으로 가면 상자 오른쪽 끝이 64에서 멈춰야 해요.
TEST(TileCollision, StopsFlushAgainstWall) {
    const auto map = make_map();
    // 발 (40, 84): 상자 x 33~47, y 76~84 -> 2번째 줄(64~96)이에요.
    const auto result = move_and_slide(map, kBody, {40.0f, 84.0f}, {40.0f, 0.0f}, 0.0f);
    EXPECT_TRUE(result.hit_x);
    EXPECT_NEAR(result.feet.x + kBody.width * 0.5f, 64.0f, 1.0f);
    EXPECT_LE(result.feet.x + kBody.width * 0.5f, 64.0f);
}

// 대각선으로 벽에 부딪히면 막힌 가로만 멈추고 세로는 계속 가요 (미끄러지기).
TEST(TileCollision, SlidesAlongWall) {
    const auto map = make_map();
    const auto result = move_and_slide(map, kBody, {55.0f, 84.0f}, {10.0f, 10.0f}, 0.0f);
    EXPECT_TRUE(result.hit_x);
    EXPECT_FALSE(result.hit_y);
    EXPECT_FLOAT_EQ(result.feet.y, 94.0f);
}

// 맵 밖은 벽이에요. 왼쪽 끝을 넘어갈 수 없어요.
TEST(TileCollision, MapEdgeBlocks) {
    const auto map = make_map();
    const auto result = move_and_slide(map, kBody, {10.0f, 20.0f}, {-30.0f, 0.0f}, 0.0f);
    EXPECT_TRUE(result.hit_x);
    EXPECT_GE(result.feet.x - kBody.width * 0.5f, 0.0f);
}

// 벽 모서리에 2픽셀만 걸렸으면, 모서리 보정이 위로 살짝 비켜서 지나가게 해 줘요.
// 벽 (2,2)의 위쪽 끝은 y = 64. 발 y = 66이면 상자 y 58~66 -> 아래 2픽셀이 걸려요.
TEST(TileCollision, CornerAssistSlipsAroundCorner) {
    const auto map = make_map();
    Vec2 feet{50.0f, 66.0f};
    bool hit = false;
    for (int i = 0; i < 30; ++i) {  // 반 초 동안 오른쪽으로 걷기 (박자마다 2픽셀)
        const auto r = move_and_slide(map, kBody, feet, {2.0f, 0.0f}, 6.0f);
        feet = r.feet;
        hit = hit || r.hit_x;
    }
    EXPECT_FALSE(hit);
    EXPECT_GT(feet.x, 64.0f + kBody.width);  // 벽 모서리를 지나갔어요
    EXPECT_LE(feet.y, 64.0f);                // 위로 비켜서 갔어요
}

// 모서리 보정이 0이면 똑같은 상황에서 그냥 막혀요.
TEST(TileCollision, NoCornerAssistBlocks) {
    const auto map = make_map();
    const auto result = move_and_slide(map, kBody, {50.0f, 66.0f}, {10.0f, 0.0f}, 0.0f);
    EXPECT_TRUE(result.hit_x);
}

// 아주 빨리 움직여도(한 박자에 100픽셀) 벽을 뚫고 지나가지 않아요.
TEST(TileCollision, FastMoveDoesNotTunnel) {
    const auto map = make_map();
    const auto result = move_and_slide(map, kBody, {40.0f, 84.0f}, {100.0f, 0.0f}, 0.0f);
    EXPECT_TRUE(result.hit_x);
    EXPECT_LE(result.feet.x + kBody.width * 0.5f, 64.0f);
}

}  // namespace
