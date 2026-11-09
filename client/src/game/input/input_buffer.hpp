#pragma once

// 이 파일은 무엇을 하나요?
// 입력 버퍼 = 버튼을 "잠깐 기억해 주는 메모장"이에요. (명세 5.8)
// 예: 구르기가 끝나기 직전에 또 구르기를 누르면, 그 순간엔 못 굴러요.
// 그래도 메모장이 0.15초 동안 기억해 뒀다가, 구를 수 있게 되는 순간 바로 굴러 줘요.
// 그래서 버튼을 "살짝 일찍" 눌러도 씹히지 않고, 조작이 손에 착 붙어요.

#include "game/input/action.hpp"

#include <array>

namespace wind::game {

class InputBuffer {
public:
    // window_seconds: 몇 초 동안 기억할지
    explicit InputBuffer(double window_seconds);

    // 버튼이 눌렸어요. 메모장에 적어요 (이미 적혀 있으면 시간을 다시 채워요).
    void press(Action action);
    // 시간이 흘렀어요. 오래된 메모는 지워져요.
    void tick(double dt_seconds);
    // 메모가 있으면 지우고 true를 돌려줘요. (한 번 쓰면 사라져요)
    [[nodiscard]] bool consume(Action action);
    // 메모가 있는지 보기만 해요.
    [[nodiscard]] bool pending(Action action) const;
    // 모든 메모를 지워요.
    void clear();

private:
    double window_;
    std::array<double, kActionCount> remaining_{};  // 동작마다 남은 기억 시간 (0이면 없음)
};

}  // namespace wind::game
