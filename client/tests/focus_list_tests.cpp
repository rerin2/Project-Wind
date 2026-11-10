// 이 파일은 무엇을 하나요?
// UI 포커스(고른 버튼)를 숙제 검사(테스트)해요.
// - 위·아래로 옮기고, 끝에서는 반대쪽 끝으로 돌아가요
// - 꺼진 버튼은 건너뛰어요
#include "engine/ui/focus_list.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::FocusList;

TEST(FocusList, MovesAndWraps) {
    FocusList focus(3);
    EXPECT_EQ(focus.current(), 0u);
    focus.move(1);
    EXPECT_EQ(focus.current(), 1u);
    focus.move(1);
    focus.move(1);
    EXPECT_EQ(focus.current(), 0u);  // 끝 다음은 처음
    focus.move(-1);
    EXPECT_EQ(focus.current(), 2u);  // 처음 앞은 끝
}

TEST(FocusList, SkipsDisabled) {
    FocusList focus(3);
    focus.set_enabled(1, false);
    focus.move(1);
    EXPECT_EQ(focus.current(), 2u);
    // 지금 고른 버튼을 끄면 다음 켜진 버튼으로 옮겨요.
    focus.set_enabled(2, false);
    EXPECT_EQ(focus.current(), 0u);
}

TEST(FocusList, NothingEnabledStaysPut) {
    FocusList focus(2);
    focus.set_enabled(0, false);
    focus.set_enabled(1, false);
    EXPECT_FALSE(focus.any_enabled());
    focus.move(1);  // 멈추지도, 끝없이 돌지도 않아요
    EXPECT_FALSE(focus.enabled(focus.current()));
    FocusList empty(0);
    empty.move(1);
    EXPECT_EQ(empty.current(), 0u);
}

}  // namespace
