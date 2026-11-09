// 이 파일은 무엇을 하나요?
// 방향키로 "가고 싶은 방향 화살표"를 맞게 만드는지 숙제 검사(테스트)를 해요.
// - 대각선도 길이가 1이라 더 빠르지 않나
// - 반대 방향을 같이 누르면 마지막에 누른 쪽으로 가나
#include "game/input/movement_intent.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::InputState;
using wind::engine::Key;
using wind::engine::Vec2;
using wind::game::InputBindings;
using wind::game::MovementIntent;

// 한 화면을 흉내 내요: 키 상태를 바꾸고, 의도를 갱신하고, 방향을 돌려줘요.
Vec2 frame(MovementIntent& intent, InputState& input, const InputBindings& bindings, Key key,
           bool down) {
    input.begin_frame();
    input.set_key(key, down);
    intent.update(bindings, input);
    return intent.direction(bindings, input);
}

// 아무것도 안 누르면 (0, 0), 오른쪽만 누르면 (1, 0)
TEST(MovementIntent, SingleKeyGivesUnitAxis) {
    const auto bindings = InputBindings::defaults();
    MovementIntent intent;
    InputState input;
    EXPECT_EQ(intent.direction(bindings, input), (Vec2{0.0f, 0.0f}));
    EXPECT_EQ(frame(intent, input, bindings, Key::Right, true), (Vec2{1.0f, 0.0f}));
}

// 위 + 오른쪽 = 대각선. 길이는 1이어야 해요 (0.707, -0.707).
TEST(MovementIntent, DiagonalIsNormalized) {
    const auto bindings = InputBindings::defaults();
    MovementIntent intent;
    InputState input;
    frame(intent, input, bindings, Key::Up, true);
    const Vec2 dir = frame(intent, input, bindings, Key::D, true);
    EXPECT_NEAR(wind::engine::length(dir), 1.0f, 1e-5f);
    EXPECT_NEAR(dir.x, 0.70710678f, 1e-5f);
    EXPECT_NEAR(dir.y, -0.70710678f, 1e-5f);
}

// 왼쪽을 누른 채 오른쪽을 누르면 오른쪽, 오른쪽을 떼면 다시 왼쪽
TEST(MovementIntent, OppositeKeysLatestWins) {
    const auto bindings = InputBindings::defaults();
    MovementIntent intent;
    InputState input;
    EXPECT_EQ(frame(intent, input, bindings, Key::Left, true), (Vec2{-1.0f, 0.0f}));
    EXPECT_EQ(frame(intent, input, bindings, Key::Right, true), (Vec2{1.0f, 0.0f}));
    EXPECT_EQ(frame(intent, input, bindings, Key::Right, false), (Vec2{-1.0f, 0.0f}));
    EXPECT_EQ(frame(intent, input, bindings, Key::Left, false), (Vec2{0.0f, 0.0f}));
}

}  // namespace
