#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 = 먼지, 불꽃, 꽃잎처럼 잠깐 생겼다 사라지는 작은 점 하나예요. (명세 6.6)
// 이 파일은 점 하나가 가진 정보(위치, 속도, 남은 시간...)만 적어 둬요.
// 화면 연출용이라서 게임 규칙(충돌, 피해)에는 아무 영향도 주지 않아요.

#include "engine/core/math.hpp"

#include <cstdint>

namespace wind::engine {

struct Particle {
    Vec2 position;           // 맵 위 위치 (픽셀)
    Vec2 velocity;           // 1초에 얼마나 움직이는지 (픽셀/초)
    float drag = 0.0f;       // 공기 저항: 클수록 빨리 느려져요
    float gravity = 0.0f;    // 아래로 끌어당기는 힘 (픽셀/초²), 음수면 위로 떠올라요
    float lifetime = 1.0f;   // 태어나서 사라질 때까지 몇 초
    float age = 0.0f;        // 태어난 지 몇 초 됐는지
    float start_size = 4.0f; // 태어날 때 크기 (픽셀)
    float end_size = 0.0f;   // 사라질 때 크기
    std::uint8_t start_alpha = 255;  // 태어날 때 진하기, 사라질 때는 0까지 흐려져요

    // 수명의 몇 %를 살았는지 (0 = 막 태어남, 1 = 사라질 때)
    [[nodiscard]] float progress() const { return lifetime > 0.0f ? age / lifetime : 1.0f; }
};

}  // namespace wind::engine
