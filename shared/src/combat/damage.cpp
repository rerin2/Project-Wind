// 이 파일은 무엇을 하나요?
// 피해 계산 순수 함수. 순서는 damage.hpp 맨 위 설명과 같아요.

#include "wind/combat/damage.hpp"

#include <algorithm>
#include <cmath>

namespace wind::combat {
namespace {

// 음수, 무한대, NaN(숫자가 아님)은 0으로 바꿔요. 잘못된 데이터가 HP를 늘리지 못하게 해요.
float sane(float value) {
    return std::isfinite(value) && value > 0.0f ? value : 0.0f;
}

}  // namespace

float physical_attack_power(float base_power, float strength_scaling, float strength_bonus) {
    return sane(base_power) * (1.0f + sane(strength_scaling) * sane(strength_bonus));
}

DamageResult compute_damage(const DamageInput& input, const DamageRules& rules) {
    if (input.invulnerable) {
        return {};
    }
    // 규칙 숫자도 한 번 정리해요: 최소는 0 이상, 최대는 최소 이상.
    const int minimum = std::max(rules.minimum, 0);
    const int maximum = std::max(rules.maximum, minimum);

    // double로 계산해서 큰 수에서도 정밀도를 지켜요.
    const double raw = static_cast<double>(sane(input.attack_power)) * sane(input.power);
    double amount = std::max(static_cast<double>(minimum), raw - sane(input.defense));
    if (input.critical) {
        amount *= sane(rules.crit_multiplier);
    }
    // 소수점 버리기 전에 최대값으로 잘라서 int로 바꿀 때 넘치지 않게 해요.
    amount = std::clamp(std::floor(amount), static_cast<double>(minimum),
                        static_cast<double>(maximum));
    return {true, static_cast<int>(amount)};
}

}  // namespace wind::combat
