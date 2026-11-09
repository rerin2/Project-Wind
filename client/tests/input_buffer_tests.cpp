// 이 파일은 무엇을 하나요?
// 버튼 메모장(InputBuffer)이 맞게 기억하고 잊는지 숙제 검사(테스트)를 해요.
#include "game/input/input_buffer.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::Action;
using wind::game::InputBuffer;

// 누르고 0.1초 안에 꺼내면 있어요. 한 번 꺼내면 사라져요.
TEST(InputBuffer, ConsumeWithinWindowOnce) {
    InputBuffer buffer(0.15);
    buffer.press(Action::Dodge);
    buffer.tick(0.1);
    EXPECT_TRUE(buffer.consume(Action::Dodge));
    EXPECT_FALSE(buffer.consume(Action::Dodge));
}

// 0.15초가 지나면 메모가 사라져요.
TEST(InputBuffer, ExpiresAfterWindow) {
    InputBuffer buffer(0.15);
    buffer.press(Action::Dodge);
    buffer.tick(0.1);
    buffer.tick(0.06);
    EXPECT_FALSE(buffer.pending(Action::Dodge));
    EXPECT_FALSE(buffer.consume(Action::Dodge));
}

// 다른 동작의 메모는 서로 섞이지 않아요.
TEST(InputBuffer, ActionsAreIndependent) {
    InputBuffer buffer(0.15);
    buffer.press(Action::Dodge);
    EXPECT_FALSE(buffer.consume(Action::Quit));
    EXPECT_TRUE(buffer.consume(Action::Dodge));
}

}  // namespace
