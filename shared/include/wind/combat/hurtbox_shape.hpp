#pragma once

// 이 파일은 무엇을 하나요?
// 맞는 상자(허트박스) 모양이에요. 허수아비(9c)와 몬스터(11a)가 함께 써요.
// 발 위치를 기준으로 왼쪽 위 모서리가 어디이고 크기가 얼마인지 적어요 (픽셀).

namespace wind::combat {

struct HurtboxShape {
    float x = -10.0f;  // 발에서 왼쪽으로 (음수 = 왼쪽)
    float y = -30.0f;  // 발에서 위로 (음수 = 위)
    float w = 20.0f;   // 가로
    float h = 30.0f;   // 세로
};

}  // namespace wind::combat
