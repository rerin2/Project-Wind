// 이 파일은 무엇을 하나요?
// 리모컨 버튼표(InputBindings)가 맞게 연결됐는지 숙제 검사(테스트)를 해요.
#include "game/input/input_bindings.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::InputState;
using wind::engine::Key;
using wind::game::Action;
using wind::game::InputBindings;

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

// 기본 버튼표에서 Space, K, X는 모두 "구르기"예요.
TEST(InputBindings, DefaultsMapDodgeKeys) {
    const auto bindings = InputBindings::defaults();
    for (const Key key : {Key::Space, Key::K, Key::X}) {
        InputState input;
        input.set_key(key, true);
        EXPECT_TRUE(bindings.pressed(Action::Dodge, input));
    }
}

}  // namespace
