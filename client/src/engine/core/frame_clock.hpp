#pragma once

// 이 파일은 무엇을 하나요?
// 게임의 "박자 맞추는 시계"예요. 메트로놈처럼 일정한 박자(예: 1초에 60번)로
// 게임 계산을 하도록 도와줘요. (명세 5.1)
//
// 왜 필요할까요? 컴퓨터마다 화면을 그리는 속도가 달라요.
// 빠른 컴퓨터에서 캐릭터가 더 빨리 걸으면 안 되니까, 계산은 항상 같은 박자로 해요.
// 또 컴퓨터가 잠깐 멈췄다가 돌아왔을 때 밀린 계산을 한꺼번에 몰아서 하면
// 캐릭터가 순간이동하듯 튀어요. 그래서 "한 번에 따라잡는 양"에 한계를 둬요.

#include <cstdint>

namespace wind::engine {

// 고정 박자 시계 (고정 스텝 = 늘 같은 간격으로 한 번씩 계산하는 것)
class FixedStepClock {
public:
    // step_seconds: 한 박자의 길이 (예: 1/60초)
    // max_frame_seconds: 한 번에 받아 줄 수 있는 가장 긴 시간 (이보다 길면 잘라 버려요)
    // max_steps: 한 번에 몰아서 계산할 수 있는 최대 박자 수
    FixedStepClock(double step_seconds, double max_frame_seconds, int max_steps);

    // "지난번부터 이만큼 시간이 흘렀어요"라고 알려 주면,
    // "그럼 지금 게임 계산을 몇 번 하세요"라고 답해 줘요.
    [[nodiscard]] int advance(double frame_seconds);

    // 한 박자의 길이
    [[nodiscard]] double step_seconds() const { return step_; }
    // 다음 박자까지 얼마나 왔는지 (0이면 방금 박자, 1에 가까우면 곧 다음 박자)
    [[nodiscard]] double alpha() const { return accumulator_ / step_; }
    // 시간이 너무 많이 밀려서 일부를 버린 적이 몇 번인지 (디버그 화면에 보여 줘요)
    [[nodiscard]] std::uint64_t dropped_frames() const { return dropped_frames_; }

private:
    double step_;       // 한 박자 길이
    double max_frame_;  // 한 번에 받아 주는 최대 시간
    int max_steps_;     // 한 번에 몰아서 하는 최대 계산 수
    double accumulator_ = 0.0;  // 아직 계산에 쓰지 않고 모아 둔 시간 (저금통 같은 것)
    std::uint64_t dropped_frames_ = 0;
};

}  // namespace wind::engine
