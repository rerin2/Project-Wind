// 이 파일은 무엇을 하나요?
// 무거운 무기: 공격 시간과 스태미나를 늘려요 (명세 8.4).

#include "wind/combat/heavy_weapon.hpp"

namespace wind::combat {

Weapon wield(Weapon weapon, bool heavy) {
    if (!heavy) return weapon;  // 들 만하면 그대로
    weapon.stamina_per_attack *= kHeavyStaminaScale;
    for (auto& attack : weapon.combo) {
        // 시간표 전체를 같은 배수로 늘려야 취소 구간이 후딜과 계속 맞아요
        attack.windup *= kHeavyTimeScale;
        attack.active *= kHeavyTimeScale;
        attack.recovery *= kHeavyTimeScale;
        attack.cancel_start *= kHeavyTimeScale;
        attack.cancel_end *= kHeavyTimeScale;
    }
    return weapon;
}

}  // namespace wind::combat
