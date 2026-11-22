// 이 파일은 무엇을 하나요?
// 파생값 → 주인공 몸 숫자 묶음 (최대값만 바꿔요).

#include "game/progression/hero_tuning.hpp"

namespace wind::game {

HealthTuning health_tuning(const progression::DerivedStats& derived) {
    HealthTuning tuning;              // 나머지(바닥 0)는 기본값
    tuning.max = derived.max_hp;
    return tuning;
}

ManaTuning mana_tuning(const progression::DerivedStats& derived) {
    ManaTuning tuning;                // 회복 1초에 2는 기본값
    tuning.max = static_cast<float>(derived.max_mp);
    return tuning;
}

StaminaTuning stamina_tuning(const progression::DerivedStats& derived) {
    StaminaTuning tuning;             // 소모·회복·지침 25는 기본값
    tuning.max = static_cast<float>(derived.max_stamina);
    return tuning;
}

}  // namespace wind::game
