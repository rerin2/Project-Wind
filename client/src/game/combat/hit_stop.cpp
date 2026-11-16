// 이 파일은 무엇을 하나요?
// 히트스톱 남은 시간을 늘리고 줄이는 곳이에요.

#include "game/combat/hit_stop.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kMaxHitStopSeconds = 0.5f;  // 데이터 상한과 같아요. 이상한 값이 와도 오래 멈추지 않게
// 1/60초를 여러 번 빼면 아주 작은 찌꺼기가 남아 한 박자 더 멈출 수 있어서 작은 여유를 둬요
constexpr float kTimeEpsilon = 1e-4f;

}  // namespace

void HitStop::trigger(float seconds) {
    if (!std::isfinite(seconds) || seconds <= 0.0f) {
        return;  // 멈춤 없는 공격
    }
    remaining_ = std::max(remaining_, std::min(seconds, kMaxHitStopSeconds));  // 더 긴 쪽 (더하지 않아요)
}

void HitStop::update(float real_dt_seconds) {
    if (real_dt_seconds <= 0.0f) {
        return;
    }
    remaining_ -= real_dt_seconds;
    if (remaining_ <= kTimeEpsilon) {
        remaining_ = 0.0f;  // 다 멈췄어요
    }
}

}  // namespace wind::game
