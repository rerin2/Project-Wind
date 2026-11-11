#pragma once

// 이 파일은 무엇을 하나요?
// 피해 계산을 하는 "순수 함수" 모음이에요. (명세 8.2, 9.3)
// 순수 함수 = 같은 숫자를 넣으면 언제나 같은 답이 나오는 계산기. 주사위도, 게임 상태도 안 봐요.
// 그래서 경계값(0, 음수, 아주 큰 수)을 테스트하기 쉬워요.
//
// 계산 순서 (docs/GAMEPLAY.md "Damage" 절과 같아요)
//   1) 맞는 쪽이 무적이면 -> 맞지 않음 (피해 0, hit = false)
//   2) 이상한 숫자(음수, 무한대, NaN)는 0으로 봐요
//   3) raw = 공격력 × 위력
//   4) raw - 방어력, 그래도 최소 피해보다 작으면 최소 피해
//   5) 치명타면 × 치명타 배수
//   6) 소수점은 버리고, 최소 피해 ~ 최대 피해 사이로 자르기

namespace wind::combat {

// 피해 규칙 숫자 (데이터로 바꿀 수 있게 한곳에 모아요). 값의 이유는 docs/GAMEPLAY.md에 있어요.
struct DamageRules {
    int minimum = 1;              // 맞으면 적어도 이만큼은 깎여요
    int maximum = 99999;          // 한 번에 이보다 많이 깎이지 않아요 (정수 넘침 방지)
    float crit_multiplier = 1.5f; // 치명타 배수
};

struct DamageInput {
    float attack_power = 0.0f;  // 공격하는 쪽 공격력 (물리 또는 마법)
    float power = 1.0f;         // 공격 데이터의 위력 배수
    float defense = 0.0f;       // 맞는 쪽 방어력
    bool critical = false;      // 치명타인가 (주사위는 부르는 쪽이 굴려요)
    bool invulnerable = false;  // 맞는 쪽이 무적인가 (구르기, 피격 직후)
};

struct DamageResult {
    bool hit = false;  // 실제로 맞았나 (무적이면 false)
    int amount = 0;    // 깎을 HP
};

// 물리 공격력 = 무기 기본 공격력 × (1 + 무기 힘 보정 × 힘 보너스)  (명세 8.2)
// 이상한 숫자(음수, NaN)는 0으로 봐요.
[[nodiscard]] float physical_attack_power(float base_power, float strength_scaling,
                                          float strength_bonus);

// 피해 한 번을 계산해요.
[[nodiscard]] DamageResult compute_damage(const DamageInput& input, const DamageRules& rules = {});

}  // namespace wind::combat
