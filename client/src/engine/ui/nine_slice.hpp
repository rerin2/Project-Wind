#pragma once

// 이 파일은 무엇을 하나요?
// 9조각(9-slice) 창 테두리 계산이에요. (명세 6.9)
// 작은 테두리 그림 하나를 # 모양으로 9조각 내서, 네 모서리는 그대로 두고
// 가장자리는 길게, 가운데는 넓게 늘려 붙이면 어떤 크기의 창도 테두리가 뭉개지지 않아요.
//   [모서리][  위  ][모서리]
//   [ 왼쪽 ][가운데][오른쪽]
//   [모서리][ 아래 ][모서리]
// 그리지는 않고 "그림 판의 어디를 화면 어디에 붙일지" 9쌍만 계산해요. 그래서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"

#include <array>
#include <cstdint>

namespace wind::engine {

struct SlicePiece {
    RectI source;  // 그림 판에서 오려 올 곳
    RectF dest;    // 화면에 붙일 곳
};

// source: 테두리 그림 전체, border: 테두리 두께, dest: 그릴 창 크기.
// 창이 테두리 두 개보다 작으면 테두리를 줄여서 겹치지 않게 해요. 크기가 0인 조각은 빈 네모예요.
[[nodiscard]] std::array<SlicePiece, 9> nine_slice(RectI source, std::int32_t border, RectF dest);

}  // namespace wind::engine
