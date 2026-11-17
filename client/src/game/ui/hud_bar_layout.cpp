// 이 파일은 무엇을 하나요?
// HUD 바의 줄 위치와 길이 계산이에요.

#include "game/ui/hud_bar_layout.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

// 줄 번호 × (높이 + 빈칸)만큼 첫 줄에서 내려가요. (HP 12, MP 21, 스태미나 30)
float hud_bar_y(HudBarRow row) {
    const auto index = static_cast<float>(static_cast<int>(row));
    return kHudBarTop + index * (kHudBarHeight + kHudBarGap);
}

// 최대값 × 0.8을 범위 안으로 자르고 정수 픽셀로 반올림해요 (픽셀이 반 칸 걸리지 않게).
float hud_bar_inner_width(float max_points) {
    if (!std::isfinite(max_points)) {
        return kHudBarInnerMin;  // NaN·무한대가 와도 화면이 깨지지 않게
    }
    const float wanted = max_points * kHudPixelsPerPoint;
    return std::round(std::clamp(wanted, kHudBarInnerMin, kHudBarInnerMax));
}

}  // namespace wind::game
