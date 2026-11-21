#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 HP·MP·스태미나 "지금 값" 묶음이에요. (12c)
// 맵을 옮기면 필드를 새로 만드는데, 그때 이 값을 들고 가서 새 필드의 주인공에게 넘겨요.
// 그래서 맵을 오가도 HP가 다시 차지 않아요. 부활할 때만 가득 찬 채 시작해요.
// (16단계 저장도 이 값을 쓰면 돼요.) 그냥 숫자 묶음이라 SDL을 몰라요.

namespace wind::game {

struct HeroVitals {
    int hp = 0;            // 지금 HP
    float mana = 0.0f;     // 지금 MP
    float stamina = 0.0f;  // 지금 스태미나
    bool exhausted = false;  // 지침 상태인가 (스태미나를 다 써서 25까지 차기를 기다리는 중)
};

}  // namespace wind::game
