#pragma once

// 이 파일은 무엇을 하나요?
// 훈련용 허수아비 설명(데이터)이에요. (명세 9단계 "HP를 가진 훈련용 허수아비")
// 숫자는 content/props/training_dummy.json에서 읽어요. 코드에 숫자를 박아 두지 않아요.

#include "wind/combat/hurtbox_shape.hpp"
#include "wind/combat/sprite_strip.hpp"

#include <string>

namespace wind::combat {

struct TrainingDummyData {
    std::string id;                // "training_dummy"
    std::string name;              // "훈련용 허수아비"
    int max_hp = 100;              // 가득 찬 HP
    float defense = 0.0f;          // 방어력 (피해 계산 9.3)
    HurtboxShape hurtbox;          // 맞는 상자
    float recover_seconds = 1.5f;  // HP가 0이 되면 쓰러져 있다가 다시 서기까지 (초)
    SpriteStrip sprite;            // 그림: 0 = 서 있음, 1 = 쓰러짐
};

}  // namespace wind::combat
