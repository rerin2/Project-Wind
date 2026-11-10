#pragma once

// 이 파일은 무엇을 하나요?
// 캐릭터가 얼마나 빨리 걷고, 얼마나 빨리 멈추고, 얼마나 멀리 구르는지 정하는 숫자 모음이에요.
// 숫자를 코드 여기저기에 흩어 두지 않고 한곳에 모아 두면 손맛을 조절하기 쉬워요.
// 값의 이유는 docs/GAMEPLAY.md "이동" 절에 적어 둬요.
// (나중 단계에서 캐릭터 자료 파일(JSON)로 옮길 거예요.)

#include "game/physics/tile_collision.hpp"

namespace wind::game {

struct MovementTuning {
    float max_speed = 110.0f;      // 걷기 최고 속도 (픽셀/초). 약 3.4칸/초
    float acceleration = 1100.0f;  // 출발할 때 빨라지는 정도 (픽셀/초²). 약 0.1초면 최고 속도
    float deceleration = 1400.0f;  // 키를 뗐을 때 느려지는 정도. 출발보다 조금 빨리 멈춰요
    float run_multiplier = 1.5f;   // 달리기(Shift) 속도 = 걷기 최고 속도 × 이 값 (165 픽셀/초)

    float dodge_speed = 280.0f;          // 구르기 속도 (픽셀/초)
    float dodge_seconds = 0.20f;         // 구르기 시간. 280 × 0.2 = 약 56픽셀(2칸 조금 안 됨)
    float dodge_cooldown_seconds = 0.30f;  // 구르기가 끝나고 다시 구를 수 있을 때까지
    float dodge_invulnerable_seconds = 0.16f;  // 구르기 시작부터 무적 시간 (단계 4 전투에서 씀)

    float corner_assist = 6.0f;  // 모서리 보정 (픽셀)
    BodyShape body;              // 발 상자 크기
};

}  // namespace wind::game
