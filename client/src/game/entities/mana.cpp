// 이 파일은 무엇을 하나요?
// MP를 쓰고(모자라면 안 씀), 천천히 다시 채우는 규칙이에요.

#include "game/entities/mana.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

// 처음에는 가득 찬 채로 시작해요. 최대가 음수로 잘못 오면 0으로 봐요.
Mana::Mana(ManaTuning tuning) : tuning_(tuning), current_(std::max(0.0f, tuning.max)) {
    tuning_.max = current_;
}

// 지금 MP가 cost 이상인지만 봐요.
bool Mana::can_afford(float cost) const {
    return current_ >= cost;
}

// 전부 아니면 전부 안 함: 모자라면 하나도 안 줄여요.
bool Mana::try_spend(float cost) {
    if (cost <= 0.0f) {
        return true;  // 공짜 동작 (음수로 몰래 채우기는 막아요)
    }
    if (!can_afford(cost)) {
        return false;
    }
    current_ -= cost;
    return true;
}

// 1초에 regen_per_second씩, 최대에서 멈춰요.
void Mana::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    current_ = std::min(tuning_.max, current_ + tuning_.regen_per_second * dt_seconds);
}

void Mana::refill() {
    current_ = tuning_.max;
}

void Mana::restore(float mana) {
    if (!std::isfinite(mana)) {
        return;  // 이상한 값은 무시
    }
    current_ = std::clamp(mana, 0.0f, tuning_.max);
}

// 최대가 0이면 나누지 않고 0이에요.
float Mana::fraction() const {
    return tuning_.max > 0.0f ? current_ / tuning_.max : 0.0f;
}

}  // namespace wind::game
