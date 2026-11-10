#pragma once

// 이 파일은 무엇을 하나요?
// 아이리스 전환(명세 6.8)의 "검은 가리개"를 계산하는 도우미 함수들이에요.
// 화면에 둥근 구멍 하나만 남기고 나머지를 까맣게 칠하면, 구멍이 작아질수록 화면이 닫혀 보여요.
// 렌더러는 동그라미를 직접 못 그려서, 한 줄(가로 1픽셀 높이)마다
// "구멍 왼쪽"과 "구멍 오른쪽"을 검은 네모로 칠해요. 줄마다 정확히 계산해서 도트처럼 또렷해요.
// 그리는 일은 하지 않고 네모 목록만 만들어요. 그래서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"

#include <vector>

namespace wind::engine {

// 덮인 정도(coverage 0~1)에 맞는 구멍 반지름.
// 0이면 구멍이 화면 네 모서리를 다 덮을 만큼 크고(아무것도 안 가림), 1이면 0(전부 까맘)이에요.
[[nodiscard]] float iris_radius(Vec2 center, Vec2 screen_size, float coverage);

// center를 가운데로 하는 radius 반지름 구멍 바깥을 덮는 검은 네모들.
// 구멍에 안 걸리는 위·아래 줄은 큰 네모 하나로 합쳐요 (그리기 횟수를 줄이려고).
[[nodiscard]] std::vector<RectF> iris_cover_rects(Vec2 center, float radius, Vec2 screen_size);

}  // namespace wind::engine
