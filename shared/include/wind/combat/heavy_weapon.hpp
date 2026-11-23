#pragma once

// 이 파일은 무엇을 하나요?
// "무거움" 상태의 무기를 만들어요. (명세 8.4, 15a)
// 무기가 들 수 있는 무게보다 무거우면(무거운지는 derive_stats가 정해요) 장착은 되지만:
//   - 공격 구간 시간(준비·판정·후딜, 취소 구간)이 1.3배로 길어지고
//   - 한 번 휘두를 때 스태미나가 1.5배 들어요.
// 공격력·넉백 같은 나머지는 그대로예요. 무겁지 않으면 무기를 그대로 돌려줘요.
// 순수 함수라 SDL을 몰라요.

#include "wind/combat/weapon.hpp"

namespace wind::combat {

inline constexpr float kHeavyTimeScale = 1.3f;     // 무거울 때 공격 시간 배수 (명세 8.4)
inline constexpr float kHeavyStaminaScale = 1.5f;  // 무거울 때 공격 스태미나 배수 (명세 8.4)

// 손에 든 무기: heavy면 시간·스태미나를 늘린 사본, 아니면 그대로
[[nodiscard]] Weapon wield(Weapon weapon, bool heavy);

}  // namespace wind::combat
