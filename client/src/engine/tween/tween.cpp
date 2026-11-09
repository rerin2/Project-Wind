// 이 파일은 무엇을 하나요?
// 트윈이 시간을 세고, 지금 값을 계산하는 곳이에요.

#include "engine/tween/tween.hpp"

#include <algorithm>

namespace wind::engine {

Tween::Tween(float from, float to, float duration_seconds, Ease ease)
    : from_(from), to_(to), duration_(std::max(duration_seconds, 0.0f)), ease_(ease) {}

void Tween::restart() {
    elapsed_ = 0.0f;
}

void Tween::update(float dt_seconds) {
    elapsed_ = std::min(duration_, elapsed_ + std::max(dt_seconds, 0.0f));
}

float Tween::progress() const {
    return duration_ > 0.0f ? elapsed_ / duration_ : 1.0f;
}

float Tween::value() const {
    // 시작 값 + (끝 값 - 시작 값) × 이징을 거친 진행률
    return from_ + (to_ - from_) * ease(ease_, progress());
}

}  // namespace wind::engine
