// 이 파일은 무엇을 하나요?
// 게임 입력 막기(InputGate, 15e)를 숙제 검사(테스트)해요: 아무것도 안 열렸으면 통과, 하나라도 열리면 막혀요.
// 완료 기준 "메뉴가 열리면 게임 입력이 막힘"의 앱 쪽 절반이에요 (메뉴 쪽은 PauseMenu.CapturesInputOnlyWhileOpen).
#include "app/input_gate.hpp"

#include <gtest/gtest.h>

namespace {

using wind::client::InputGate;

TEST(InputGate, NothingOpenLetsInputThrough) {
    EXPECT_FALSE(InputGate{}.blocks_game());
}

TEST(InputGate, PauseMenuBlocksGameInput) {
    InputGate gate;
    gate.pause_menu = true;
    EXPECT_TRUE(gate.blocks_game());
}

TEST(InputGate, EveryOtherScreenBlocksToo) {
    EXPECT_TRUE((InputGate{true, false, false, false}.blocks_game()));   // 맵 옮기기
    EXPECT_TRUE((InputGate{false, true, false, false}.blocks_game()));   // F7 시험판
    EXPECT_TRUE((InputGate{false, false, true, false}.blocks_game()));   // 쿠션 메뉴
}

}  // namespace
