#pragma once

// 이 파일은 무엇을 하나요?
// HUD 왼쪽 위 바(HP·MP·스태미나)의 자리와 길이를 계산하는 작은 함수들이에요. (명세 13.2)
// - 순서: 맨 위 HP, 그 아래 MP, 그 아래 스태미나 (명세 13.2 "HP·MP·스태미나")
// - 길이: 최대값 1당 0.8픽셀 (HP 100 → 80픽셀). 능력치를 올려 최대값이 늘면 바가 길어져요.
// 그리지 않고 숫자만 계산해서 창 없이 테스트할 수 있어요. 값의 이유는 docs/VISUALS.md "HUD and text" 절.

namespace wind::game {

// HUD 바 줄 번호 (위에서부터)
enum class HudBarRow { Health = 0, Mana = 1, Stamina = 2 };

// 바 모양 숫자 (테두리 포함 높이 7 = 테두리 1 + 안쪽 5 + 테두리 1)
inline constexpr float kHudBarLeft = 12.0f;           // 화면 왼쪽에서
inline constexpr float kHudBarTop = 12.0f;            // 첫 줄(HP)이 화면 위에서
inline constexpr float kHudBarHeight = 7.0f;          // 테두리 포함 높이
inline constexpr float kHudBarGap = 2.0f;             // 줄 사이 빈칸
inline constexpr float kHudPixelsPerPoint = 0.8f;     // 최대값 1당 안쪽 길이
inline constexpr float kHudBarInnerMin = 8.0f;        // 최대값이 아주 작아도 보이게
inline constexpr float kHudBarInnerMax = 200.0f;      // 이상한 값이 와도 화면을 넘지 않게

// 그 줄 바의 위쪽 y (테두리 포함)
[[nodiscard]] float hud_bar_y(HudBarRow row);

// 최대값으로 바 안쪽 길이(픽셀, 정수)를 구해요. 8~200으로 잘라요. 음수·NaN이면 가장 짧게.
[[nodiscard]] float hud_bar_inner_width(float max_points);

}  // namespace wind::game
