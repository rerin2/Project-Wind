#pragma once

// 이 파일은 무엇을 하나요?
// 키 바인딩(리모컨 버튼마다 기능을 정해 두는 것)을 맡아요.
// 예: "위로 가기" 동작 = 위 방향키 또는 W 키
// 그래서 나중에 키를 바꿔도 게임 코드는 그대로 둘 수 있어요. (명세 5.8)
// 바꾼 키를 저장하는 기능은 나중 단계에서 만들어요.

#include "engine/input/input_state.hpp"
#include "game/input/action.hpp"

#include <array>
#include <vector>

namespace wind::game {

// "동작 ↔ 키" 연결표
class InputBindings {
public:
    // 기본 바인딩: 방향키/WASD 이동, Space/K/X 구르기, F3 디버그, F4 충돌 표시, F5 시간, F6 날씨, F12 스크린샷, Esc 종료
    [[nodiscard]] static InputBindings defaults();

    // 동작에 키를 하나 더 연결해요. (한 동작에 키 여러 개 가능)
    void bind(Action action, engine::Key key);
    // 동작에 연결된 키를 모두 지워요.
    void clear(Action action);

    // 이 동작에 연결된 키 중 하나라도 "지금 누르고 있으면" true
    [[nodiscard]] bool held(Action action, const engine::InputState& input) const;
    // 이 동작에 연결된 키 중 하나라도 "방금 눌렀으면" true
    [[nodiscard]] bool pressed(Action action, const engine::InputState& input) const;

private:
    // 동작 번호마다 연결된 키 목록을 담는 서랍장
    std::array<std::vector<engine::Key>, kActionCount> keys_;
};

}  // namespace wind::game
