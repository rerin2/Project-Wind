// 이 파일은 무엇을 하나요?
// HP를 깎고, 가득 채우고, 비율을 계산하는 곳이에요.

#include "game/entities/health.hpp"

#include <algorithm>

namespace wind::game {

Health::Health(HealthTuning tuning) : tuning_(tuning), current_(tuning.max) {
    tuning_.max = std::max(1, tuning_.max);                       // 최대는 적어도 1
    tuning_.floor = std::clamp(tuning_.floor, 0, tuning_.max);     // 바닥은 0~최대
    current_ = tuning_.max;                                        // 가득 찬 채 시작
}

int Health::take(int amount) {
    if (amount <= 0) return 0;                                       // 음수·0 피해는 무시
    const int before = current_;
    current_ = std::max(tuning_.floor, current_ - amount);          // 바닥 아래로는 안 내려가요
    return before - current_;                                        // 실제로 깎인 양
}

void Health::restore(int hp) {
    current_ = std::clamp(hp, tuning_.floor, tuning_.max);  // 이상한 값도 범위 안으로
}

void Health::set_max(int max) {
    tuning_.max = std::max(max, 1);                          // 0으로 나누지 않게 최소 1
    current_ = std::clamp(current_, tuning_.floor, tuning_.max);  // 넘치면 새 최대로
}

float Health::fraction() const {
    return static_cast<float>(current_) / static_cast<float>(tuning_.max);
}

}  // namespace wind::game
