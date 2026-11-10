// 이 파일은 무엇을 하나요?
// 아이리스 구멍 크기와, 구멍 바깥을 덮는 검은 네모들을 계산해요.

#include "engine/fx/iris_mask.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {

float iris_radius(Vec2 center, Vec2 screen_size, float coverage) {
    // 가운데에서 가장 먼 모서리까지의 거리 = 화면을 다 보여 주는 구멍 크기
    const float far_x = std::max(center.x, screen_size.x - center.x);
    const float far_y = std::max(center.y, screen_size.y - center.y);
    const float full = std::sqrt(far_x * far_x + far_y * far_y);
    return full * (1.0f - std::clamp(coverage, 0.0f, 1.0f));
}

std::vector<RectF> iris_cover_rects(Vec2 center, float radius, Vec2 screen_size) {
    std::vector<RectF> rects;
    const int rows = static_cast<int>(std::ceil(screen_size.y));
    // 구멍에 걸리는 줄: 가운데 줄에서 반지름만큼 위·아래 (화면 밖은 잘라요)
    const int first = std::clamp(static_cast<int>(std::floor(center.y - radius)), 0, rows);
    const int last = std::clamp(static_cast<int>(std::ceil(center.y + radius)), first, rows);

    // 구멍보다 위쪽 줄들과 아래쪽 줄들은 통째로 까맣게
    if (first > 0) {
        rects.push_back({0.0f, 0.0f, screen_size.x, static_cast<float>(first)});
    }
    for (int y = first; y < last; ++y) {
        // 이 줄 한가운데(y + 0.5)에서 구멍의 반쪽 너비: 피타고라스 정리 (가로² + 세로² = 반지름²)
        const float dy = static_cast<float>(y) + 0.5f - center.y;
        const float half_sq = radius * radius - dy * dy;
        const float half = half_sq > 0.0f ? std::sqrt(half_sq) : 0.0f;
        // 정수 픽셀로 맞춰서 도트 그림처럼 또렷하게
        const float left = std::clamp(std::round(center.x - half), 0.0f, screen_size.x);
        const float right = std::clamp(std::round(center.x + half), left, screen_size.x);
        const auto row = static_cast<float>(y);
        if (left > 0.0f) {
            rects.push_back({0.0f, row, left, 1.0f});
        }
        if (right < screen_size.x) {
            rects.push_back({right, row, screen_size.x - right, 1.0f});
        }
    }
    if (last < rows) {
        rects.push_back({0.0f, static_cast<float>(last), screen_size.x,
                         screen_size.y - static_cast<float>(last)});
    }
    return rects;
}

}  // namespace wind::engine
