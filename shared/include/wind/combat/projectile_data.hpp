#pragma once

// 이 파일은 무엇을 하나요?
// "날아가는 공격" 설명이에요. (명세 9.2 `projectile`, 8.6)
// 마술(별빛 조각)과 공격 데이터(몬스터의 침 뱉기 같은 것)가 같은 모양을 함께 써요.
// 규칙: 벽에 닿거나 range만큼 날아가면 사라지고, 대상 하나에 맞으면 사라져요.
// 숫자는 데이터 파일에서 읽고, 날아가는 계산은 9e의 Projectile이 해요.

namespace wind::combat {

struct ProjectileData {
    float speed = 0.0f;          // 날아가는 빠르기 (픽셀/초)
    float range = 0.0f;          // 최대 날아가는 거리 (픽셀). 넘으면 사라져요
    float width = 1.0f;          // 탄의 크기 (픽셀). 이 네모로 벽·대상과 겹치는지 봐요
    float height = 1.0f;
    int count = 1;               // 한 번에 몇 개 (별무리 5개). 9단계는 1개만 움직여요
    float homing = 0.0f;         // 유도 세기 0~1 (0 = 똑바로). 15단계에서 움직여요
    float orbit_seconds = 0.0f;  // 날아가기 전에 주위를 도는 시간 (별무리). 15단계에서 움직여요
};

}  // namespace wind::combat
