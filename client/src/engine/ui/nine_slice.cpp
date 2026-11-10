// 이 파일은 무엇을 하나요?
// 9조각 테두리의 오려 올 곳과 붙일 곳을 계산해요.

#include "engine/ui/nine_slice.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {

std::array<SlicePiece, 9> nine_slice(RectI source, std::int32_t border, RectF dest) {
    // 창이 작으면 테두리를 절반 크기까지 줄여요 (정수 픽셀).
    const float max_border = std::floor(std::min(dest.w, dest.h) * 0.5f);
    const float b = std::clamp(static_cast<float>(border), 0.0f, std::max(0.0f, max_border));
    const auto sb = std::clamp(border, 0, std::min(source.w, source.h) / 2);

    // 그림 판: 세 칸의 가로 위치·폭, 세 줄의 세로 위치·높이
    const std::int32_t sx[3] = {source.x, source.x + sb, source.x + source.w - sb};
    const std::int32_t sw[3] = {sb, source.w - sb * 2, sb};
    const std::int32_t sy[3] = {source.y, source.y + sb, source.y + source.h - sb};
    const std::int32_t sh[3] = {sb, source.h - sb * 2, sb};
    // 화면: 같은 방식으로, 가운데만 창 크기에 맞게 늘어나요.
    const float dx[3] = {dest.x, dest.x + b, dest.x + dest.w - b};
    const float dw[3] = {b, std::max(0.0f, dest.w - b * 2.0f), b};
    const float dy[3] = {dest.y, dest.y + b, dest.y + dest.h - b};
    const float dh[3] = {b, std::max(0.0f, dest.h - b * 2.0f), b};

    std::array<SlicePiece, 9> pieces{};
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            pieces[static_cast<std::size_t>(row * 3 + col)] = {
                {sx[col], sy[row], sw[col], sh[row]},
                {dx[col], dy[row], dw[col], dh[row]},
            };
        }
    }
    return pieces;
}

}  // namespace wind::engine
