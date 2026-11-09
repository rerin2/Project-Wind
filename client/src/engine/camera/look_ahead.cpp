// 이 파일은 무엇을 하나요?
// 카메라 앞보기 거리를 부드럽게 바꾸는 계산이에요.

#include "engine/camera/look_ahead.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {

LookAhead::LookAhead(float distance, float sharpness)
    : distance_(std::max(distance, 0.0f)), sharpness_(std::max(sharpness, 0.0f)) {}

void LookAhead::update(Vec2 direction, float dt_seconds) {
    // 목표 = 가는 방향 × 최대 거리
    const Vec2 target = direction * distance_;
    // 남은 거리의 일정 비율만큼만 다가가요. (카메라 follow와 같은 방식)
    // 1 - e^(-빠르기 × 시간): 화면을 몇 장 그리든 같은 빠르기로 따라가게 해 줘요.
    const float t = 1.0f - std::exp(-sharpness_ * std::max(dt_seconds, 0.0f));
    offset_ = lerp(offset_, target, t);
}

}  // namespace wind::engine
