// 이 파일은 무엇을 하나요?
// 키보드 출석부(InputState)가 맞게 동작하는지 숙제 검사(테스트)를 해요.
// - 방금 누름 / 누르고 있음 / 방금 뗌을 잘 구분하나
// - 메뉴 같은 UI가 키보드를 쓰면 게임이 키를 무시하나
// begin_frame()은 "새 화면 한 장 시작"을 흉내 내는 거예요.
#include "engine/input/input_state.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::InputState;
using wind::engine::Key;

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

}  // namespace
