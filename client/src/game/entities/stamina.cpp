// 이 파일은 무엇을 하나요?
// 스태미나를 쓰고, 기다렸다가, 다시 채우는 규칙이에요.

#include "game/entities/stamina.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

Stamina::Stamina(StaminaTuning tuning) : tuning_(tuning), current_(std::max(0.0f, tuning.max)) {}

bool Stamina::can_run() const {
    return !exhausted_ && current_ > 0.0f;
}

bool Stamina::can_afford(float cost) const {
    return !exhausted_ && current_ >= cost;
}

bool Stamina::can_attack() const {
    return !exhausted_ && current_ > 0.0f;
}

void Stamina::spend(float amount) {
    if (amount <= 0.0f) {
        return;
    }
    current_ = std::max(0.0f, current_ - amount);
    regen_wait_ = tuning_.regen_delay_seconds;
    if (current_ <= 0.0f) {
        exhausted_ = true;  // 다 썼어요: 어느 정도 찰 때까지 쉬어야 해요
    }
}

void Stamina::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    if (regen_wait_ > 0.0f) {
        // 아직 기다리는 중. 남은 시간보다 박자가 길면, 남는 시간만큼은 채워요.
        const float waited = std::min(regen_wait_, dt_seconds);
        regen_wait_ -= waited;
        dt_seconds -= waited;
    }
    current_ = std::min(tuning_.max, current_ + tuning_.regen_per_second * dt_seconds);
    if (exhausted_ && current_ >= tuning_.exhausted_until) {
        exhausted_ = false;
    }
}

void Stamina::refill() {
    current_ = tuning_.max;
    regen_wait_ = 0.0f;
    exhausted_ = false;
}

void Stamina::restore(float stamina, bool exhausted) {
    if (!std::isfinite(stamina)) {
        return;  // 이상한 값은 무시
    }
    current_ = std::clamp(stamina, 0.0f, tuning_.max);
    regen_wait_ = 0.0f;
    // 지침은 들고 온 대로. 단, 다 찼으면 지침일 수 없고, 0이면 늘 지침이에요 (spend와 같은 규칙)
    exhausted_ = current_ <= 0.0f || (exhausted && current_ < tuning_.exhausted_until);
}

float Stamina::fraction() const {
    return tuning_.max > 0.0f ? current_ / tuning_.max : 0.0f;
}

}  // namespace wind::game
