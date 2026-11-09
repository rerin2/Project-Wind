#pragma once

// 이 파일은 무엇을 하나요?
// 빛 하나의 정보예요. 횃불, 창문 불빛, 주인공이 들고 다니는 작은 빛 같은 것. (명세 6.3)
// 빛은 동그랗게 퍼지고, 가운데가 가장 밝고 가장자리로 갈수록 어두워져요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

namespace wind::engine {

struct Light {
    Vec2 position;            // 맵 위 위치 (픽셀)
    float radius = 64.0f;     // 빛이 닿는 거리 (픽셀)
    Color color{255, 255, 255, 255};
    float intensity = 1.0f;   // 밝기 0~1
    float flicker = 0.0f;     // 깜빡임 세기 0~1 (횃불처럼 불규칙하게 흔들림)
    float pulse_frequency = 0.0f;  // 숨쉬기 빠르기 (1초에 몇 번)
    float pulse_depth = 0.0f;      // 숨쉬기 세기 0~1 (마법 빛처럼 규칙적으로 밝아졌다 어두워짐)
    float phase = 0.0f;       // 빛마다 다른 출발점 (모든 횃불이 똑같이 깜빡이지 않게)
};

}  // namespace wind::engine
