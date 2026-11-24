#pragma once

// 이 파일은 무엇을 하나요?
// "지금 게임(주인공) 입력을 막아야 하나?"를 한 곳에서 정해요. (명세 13.3 입력 포커스, 15e)
// 하나라도 켜져 있으면 게임 루프가 필드에 "아무 키도 안 누름"을 넘겨서 주인공이 제자리에 서요.
// 값만 담은 작은 묶음이라 창 없이 테스트할 수 있어요 (input_gate_tests).

namespace wind::client {

struct InputGate {
    bool transition = false;    // 맵을 옮기는 중 (페이드·아이리스)
    bool test_panel = false;    // F7 UI 시험판이 열림
    bool cushion_menu = false;  // 쿠션 메뉴·레벨업 화면 (13c)
    bool pause_menu = false;    // 일시정지 메뉴 (15e, 가방 탭 포함)

    [[nodiscard]] bool blocks_game() const { return transition || test_panel || cushion_menu || pause_menu; }
};

}  // namespace wind::client
