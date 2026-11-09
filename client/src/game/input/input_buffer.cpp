// 이 파일은 무엇을 하나요?
// 입력 버퍼(버튼 메모장)가 메모를 적고, 시간이 지나면 지우고, 꺼내 쓰는 방법이에요.

#include "game/input/input_buffer.hpp"

#include <algorithm>

namespace wind::game {
namespace {

constexpr std::size_t index(Action action) {
    return static_cast<std::size_t>(action);
}

}  // namespace

InputBuffer::InputBuffer(double window_seconds) : window_(std::max(window_seconds, 0.0)) {}

void InputBuffer::press(Action action) {
    remaining_[index(action)] = window_;
}

void InputBuffer::tick(double dt_seconds) {
    for (double& r : remaining_) {
        r = std::max(0.0, r - dt_seconds);
    }
}

bool InputBuffer::consume(Action action) {
    double& r = remaining_[index(action)];
    if (r <= 0.0) {
        return false;
    }
    r = 0.0;
    return true;
}

bool InputBuffer::pending(Action action) const {
    return remaining_[index(action)] > 0.0;
}

void InputBuffer::clear() {
    remaining_.fill(0.0);
}

}  // namespace wind::game
