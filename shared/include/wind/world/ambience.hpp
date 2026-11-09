#pragma once

// 이 파일은 무엇을 하나요?
// 맵의 "분위기" 정보예요. (명세 6.3, 7.1)
// - 주변 효과: 늘 떠다니는 꽃잎·반딧불 같은 파티클 효과 파일들
// - 주변광: 맵 전체의 기본 밝기와 색. 없으면 조명을 쓰지 않고 그림 그대로 보여요.
//          있으면 맵이 그 색으로 어두워지고, 빛이 있는 곳만 밝아요 (동굴, 밤).
// - 빛: 맵 곳곳에 놓인 횃불·창문 같은 빛, 그리고 주인공이 들고 다니는 작은 빛

#include "wind/world/rgba.hpp"

#include <optional>
#include <string>
#include <vector>

namespace wind::world {

// 맵에 놓인 빛 하나
struct LightDef {
    float x = 0.0f;  // 위치 (칸 단위, 소수 가능). 10.5 = 10번 칸의 한가운데
    float y = 0.0f;
    float radius = 64.0f;     // 빛이 닿는 거리 (픽셀)
    Rgba color{255, 255, 255, 255};
    float intensity = 1.0f;   // 밝기 0~1
    float flicker = 0.0f;     // 깜빡임 세기 0~1 (횃불은 0.3 정도)
    float pulse_frequency = 0.0f;  // 숨 쉬듯 밝아졌다 어두워지는 빠르기 (1초에 몇 번)
    float pulse_depth = 0.0f;      // 숨쉬기 세기 0~1
    std::string fx;  // 빛 자리에서 계속 나올 파티클 효과 파일 (횃불 불티 등). 없으면 빈 글자
};

// 주인공을 따라다니는 작은 빛 (어두운 맵에서 발밑이 보이게)
struct PlayerLight {
    float radius = 64.0f;
    Rgba color{255, 255, 255, 255};
    float intensity = 1.0f;
};

struct Ambience {
    std::vector<std::string> ambient_fx;  // 늘 떠다니는 효과 파일 위치들
    std::optional<Rgba> ambient_light;    // 주변광. 없으면 조명 없음 (밝은 낮 맵)
    std::optional<PlayerLight> player_light;
    std::vector<LightDef> lights;
};

}  // namespace wind::world
