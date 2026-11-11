// 이 파일은 무엇을 하나요?
// 히트박스가 바라보는 방향에 맞게 돌아가는지 숙제 검사해요.
#include "game/combat/hitbox_placement.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::RectF;
using wind::game::place_hitbox;
using wind::world::Direction;

// 앞으로 18, 위로 8, 앞뒤 길이 28, 옆 폭 24
constexpr wind::combat::HitboxShape kShape{18.0f, -8.0f, 28.0f, 24.0f};
constexpr wind::engine::Vec2 kFeet{100.0f, 200.0f};

TEST(HitboxPlacement, RightIsTheWrittenShape) {
    // 가운데 (118, 192), 28 x 24
    EXPECT_EQ(place_hitbox(kShape, kFeet, Direction::Right), (RectF{104.0f, 180.0f, 28.0f, 24.0f}));
}

TEST(HitboxPlacement, LeftMirrors) {
    // 가운데 (82, 192)
    EXPECT_EQ(place_hitbox(kShape, kFeet, Direction::Left), (RectF{68.0f, 180.0f, 28.0f, 24.0f}));
}

// 위·아래는 가로세로가 바뀌고, lift(-8)는 여전히 화면 위로 옮겨요.
TEST(HitboxPlacement, VerticalSwapsSides) {
    // 아래: 가운데 (100, 200 + 18 - 8 = 210), 24 x 28
    EXPECT_EQ(place_hitbox(kShape, kFeet, Direction::Down), (RectF{88.0f, 196.0f, 24.0f, 28.0f}));
    // 위: 가운데 (100, 200 - 18 - 8 = 174)
    EXPECT_EQ(place_hitbox(kShape, kFeet, Direction::Up), (RectF{88.0f, 160.0f, 24.0f, 28.0f}));
}

}  // namespace
