// 이 파일은 무엇을 하나요?
// 피해 숫자를 띄우고, 튀어 오르게 하고, 흐려지게 하는 계산이에요.

#include "game/presentation/damage_numbers.hpp"

#include <algorithm>

namespace wind::game {
namespace {

// 좌우 비키기 순서: 가운데, 오른쪽, 왼쪽, 가운데 ... (×side_step)
constexpr float kSidePattern[] = {0.0f, 1.0f, -1.0f};
constexpr std::size_t kSidePatternSize = sizeof(kSidePattern) / sizeof(kSidePattern[0]);
// 오르는 모양: 앞의 이만큼(35%) 동안 다 오르고 그 뒤로는 그 높이에 머물러요
constexpr float kRisePart = 0.35f;

}  // namespace

DamageNumbers::DamageNumbers(DamageNumberTuning tuning) : tuning_(tuning) {}

void DamageNumbers::spawn(engine::Vec2 at, int amount, bool critical) {
    if (numbers_.size() >= tuning_.max_numbers && !numbers_.empty()) {
        numbers_.erase(numbers_.begin());  // 너무 많으면 가장 오래된 것부터
    }
    const float side = kSidePattern[spawned_ % kSidePatternSize] * tuning_.side_step;
    ++spawned_;
    numbers_.push_back({at, amount, critical, side, 0.0f});
}

void DamageNumbers::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    for (auto& number : numbers_) {
        number.age += dt_seconds;
    }
    std::erase_if(numbers_, [this](const DamageNumber& n) { return n.age >= life_of(n); });  // 다 산 숫자
}

engine::Vec2 DamageNumbers::position(const DamageNumber& number) const {
    const float t = std::clamp(number.age / (life_of(number) * kRisePart), 0.0f, 1.0f);  // 오르기 0~1
    const float eased = 1.0f - (1.0f - t) * (1.0f - t);  // 처음엔 빠르게, 끝에서 천천히 (공을 던진 듯)
    const float rise = number.critical ? tuning_.crit_rise : tuning_.rise;
    return {number.at.x + number.side, number.at.y - rise * eased};
}

float DamageNumbers::alpha(const DamageNumber& number) const {
    const float t = number.age / life_of(number);  // 0(처음) ~ 1(끝)
    if (t <= tuning_.fade_from) {
        return 1.0f;
    }
    return std::clamp(1.0f - (t - tuning_.fade_from) / (1.0f - tuning_.fade_from), 0.0f, 1.0f);
}

float DamageNumbers::life_of(const DamageNumber& number) const {
    return number.critical ? tuning_.crit_life : tuning_.life;
}

}  // namespace wind::game
