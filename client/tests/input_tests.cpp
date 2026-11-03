// 이 파일은 무엇을 하나요?
// 키보드 관련 장치들이 맞게 동작하는지 숙제 검사(테스트)를 해요.
// - 키보드 출석부: 방금 누름 / 누르고 있음 / 방금 뗌을 잘 구분하나
// - 채팅창 같은 UI가 키보드를 쓰면 게임이 키를 무시하나
// - 리모컨 버튼표(키 바인딩)가 맞게 연결됐나
// - 방향키를 여러 개 누르면 마지막에 누른 쪽으로 가나
// begin_frame()은 "새 화면 한 장 시작"을 흉내 내는 거예요.
#include "engine/input/input_state.hpp"
#include "game/input/input_bindings.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::InputState;
using wind::engine::Key;
using wind::game::Action;
using wind::game::InputBindings;
using wind::world::Direction;

// "방금 누름"(pressed)은 그 한 장에서만 참이고, "누르고 있음"(held)은 뗄 때까지 계속 참이에요.
TEST(InputState, TracksEdgesPerFrame) {
    InputState input;
    input.begin_frame();
    input.set_key(Key::A, true);
    EXPECT_TRUE(input.pressed(Key::A));
    EXPECT_TRUE(input.held(Key::A));

    input.begin_frame();
    EXPECT_FALSE(input.pressed(Key::A));
    EXPECT_TRUE(input.held(Key::A));

    input.set_key(Key::A, false);
    EXPECT_TRUE(input.released(Key::A));
    EXPECT_FALSE(input.held(Key::A));
}

// 이미 누르고 있는 키를 "또 눌렸어요"라고 알려 줘도, 새로 누른 것으로 치지 않아요.
TEST(InputState, DuplicateDownIsNotANewPress) {
    InputState input;
    input.set_key(Key::W, true);
    input.begin_frame();
    input.set_key(Key::W, true);
    EXPECT_FALSE(input.pressed(Key::W));
}

// 다른 창을 보러 가면(포커스를 잃으면) 누르고 있던 키가 모두 뗀 것으로 바뀌어야 해요.
TEST(InputState, ReleaseAllClearsHeldKeys) {
    InputState input;
    input.set_key(Key::Up, true);
    input.release_all();
    EXPECT_FALSE(input.held(Key::Up));
    EXPECT_TRUE(input.released(Key::Up));
}

// 채팅창 같은 UI가 키보드를 쓰는 동안에는 게임이 키를 못 본 척해야 해요. (명세 5.8)
// UI가 끝나면 다시 키가 보여야 해요.
TEST(InputState, UiCaptureBlocksGameQueries) {
    InputState input;
    input.set_key(Key::D, true);
    input.set_ui_capture(true);
    EXPECT_FALSE(input.held(Key::D));
    EXPECT_FALSE(input.pressed(Key::D));
    input.set_ui_capture(false);
    EXPECT_TRUE(input.held(Key::D));
}

// 기본 버튼표에서는 A 키와 왼쪽 방향키 둘 다 "왼쪽으로 가기"여야 해요.
TEST(InputBindings, DefaultsMapArrowsAndWasd) {
    const auto bindings = InputBindings::defaults();
    InputState input;
    input.set_key(Key::A, true);
    EXPECT_TRUE(bindings.held(Action::MoveLeft, input));
    input.set_key(Key::A, false);
    input.set_key(Key::Left, true);
    EXPECT_TRUE(bindings.held(Action::MoveLeft, input));
    EXPECT_FALSE(bindings.held(Action::MoveRight, input));
}

// "끄기" 버튼을 Esc에서 Enter로 바꾸면, Esc는 더 이상 안 되고 Enter가 돼야 해요.
TEST(InputBindings, RebindingReplacesKeys) {
    auto bindings = InputBindings::defaults();
    bindings.clear(Action::Quit);
    bindings.bind(Action::Quit, Key::Enter);
    InputState input;
    input.set_key(Key::Escape, true);
    EXPECT_FALSE(bindings.pressed(Action::Quit, input));
    input.set_key(Key::Enter, true);
    EXPECT_TRUE(bindings.pressed(Action::Quit, input));
}

// 위를 누른 채 오른쪽을 누르면 오른쪽(나중에 누른 것)으로 가야 해요.
// 오른쪽을 떼면 아직 누르고 있는 위로 돌아가고, 다 떼면 "가고 싶은 방향 없음"이에요.
TEST(MovementIntent, MostRecentDirectionWins) {
    const auto bindings = InputBindings::defaults();
    wind::game::MovementIntent intent;
    InputState input;

    input.begin_frame();
    input.set_key(Key::Up, true);
    intent.update(bindings, input);
    EXPECT_EQ(intent.direction(bindings, input), Direction::Up);

    input.begin_frame();
    input.set_key(Key::Right, true);
    intent.update(bindings, input);
    EXPECT_EQ(intent.direction(bindings, input), Direction::Right);

    input.begin_frame();
    input.set_key(Key::Right, false);
    intent.update(bindings, input);
    EXPECT_EQ(intent.direction(bindings, input), Direction::Up);

    input.begin_frame();
    input.set_key(Key::Up, false);
    intent.update(bindings, input);
    EXPECT_FALSE(intent.direction(bindings, input).has_value());
}

}  // namespace
