// 이 파일은 무엇을 하나요?
// frame_clock.hpp에서 약속한 "박자 시계"가 실제로 어떻게 동작하는지 적은 곳이에요.

#include "engine/core/frame_clock.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {

// 시계를 만들 때 박자 길이와 한계값들을 기억해 둬요.
FixedStepClock::FixedStepClock(double step_seconds, double max_frame_seconds, int max_steps)
    : step_(step_seconds), max_frame_(max_frame_seconds), max_steps_(max_steps) {}

int FixedStepClock::advance(double frame_seconds) {
    // 시간이 거꾸로 흘렀다고 나오면(음수) 0으로 바꿔요.
    // 창을 끌고 있었거나 프로그램이 오래 멈춰서 시간이 너무 길면, 최대 시간까지만 인정해요.
    const double clamped = std::clamp(frame_seconds, 0.0, max_frame_);
    if (clamped < frame_seconds) {
        ++dropped_frames_;  // 잘라 버린 시간이 있었다고 기록
    }
    accumulator_ += clamped;  // 흐른 시간을 저금통에 넣어요

    // 저금통에 한 박자만큼 시간이 모일 때마다 계산 한 번을 하라고 세요.
    // 단, 한 번에 max_steps번보다 많이 하지는 않아요 (몰아치기 방지).
    int steps = 0;
    while (accumulator_ >= step_ && steps < max_steps_) {
        accumulator_ -= step_;
        ++steps;
    }
    // 한계 때문에 아직도 한 박자 넘게 남았다면, 그 시간은 따라잡지 않고 버려요.
    // 한 박자보다 작은 나머지만 남겨 둬요. (fmod = 나눗셈의 나머지 구하기)
    if (accumulator_ >= step_) {
        accumulator_ = std::fmod(accumulator_, step_);
        ++dropped_frames_;
    }
    return steps;
}

}  // namespace wind::engine
