// 이 파일은 무엇을 하나요?
// 슬로모션 시계를 흘리고, 지금 시뮬레이션 빠르기 배수를 계산하는 곳이에요.

#include "game/combat/slow_motion.hpp"

#include "engine/tween/easing.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

SlowMotion::SlowMotion(SlowMotionTuning tuning) : tuning_(tuning), elapsed_(0.0f) {
    // 이상한 값이 와도 멈추거나 빨라지지 않게: 배수는 0.05~1, 시간은 0 이상
    tuning_.start_scale = std::isfinite(tuning_.start_scale) ? std::clamp(tuning_.start_scale, 0.05f, 1.0f) : 1.0f;
    tuning_.seconds = std::isfinite(tuning_.seconds) ? std::max(0.0f, tuning_.seconds) : 0.0f;
    elapsed_ = tuning_.seconds;  // 처음에는 느려지지 않아요
}

void SlowMotion::trigger() {
    elapsed_ = 0.0f;
}

void SlowMotion::update(float real_dt_seconds) {
    if (!std::isfinite(real_dt_seconds) || real_dt_seconds <= 0.0f) {
        return;  // 이상한 시간은 무시해요
    }
    elapsed_ = std::min(tuning_.seconds, elapsed_ + real_dt_seconds);
}

float SlowMotion::time_scale() const {
    if (!active()) {
        return 1.0f;
    }
    const float t = elapsed_ / tuning_.seconds;                 // 0~1 진행
    const float back = engine::ease(engine::Ease::InQuad, t);  // 처음엔 거의 그대로 느리고, 끝에서 빨리 돌아와요
    return tuning_.start_scale + (1.0f - tuning_.start_scale) * back;
}

}  // namespace wind::game
