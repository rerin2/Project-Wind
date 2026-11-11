#pragma once

// 이 파일은 무엇을 하나요?
// 무기 하나(예: 롱소드)를 적은 데이터예요. (명세 8.4)
// 무기는 "공격 데이터를 순서대로 이어 붙인 콤보"를 가져요. 롱소드는 3타예요.
// 숫자는 content/weapons/*.json에서 읽어요.

#include "wind/combat/attack.hpp"

#include <string>
#include <vector>

namespace wind::combat {

struct Weapon {
    std::string id;                  // "longsword"
    std::string name;                // 화면에 보일 이름 ("롱소드")
    float weight = 0.0f;             // 무게 (들 수 있는 무게보다 무거우면 "무거움", 단계 15)
    float base_power = 0.0f;         // 기본 공격력
    float strength_scaling = 0.0f;   // 힘 보정 (힘 보너스에 곱해요)
    float stamina_per_attack = 0.0f; // 한 번 휘두를 때 쓰는 스태미나
    float crit_bonus = 0.0f;         // 치명타 확률 더하기 (0~1, 선택)
    std::vector<AttackData> combo;   // 콤보 순서대로의 공격들 (1타, 2타, 3타 ...)
};

}  // namespace wind::combat
