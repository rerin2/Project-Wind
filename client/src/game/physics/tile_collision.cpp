// 이 파일은 무엇을 하나요?
// 발 상자를 벽에 부딪히지 않게 옮기는 방법을 실제로 적은 곳이에요.
//
// 방법: 한 번에 크게 옮기지 않고 1픽셀 이하씩 잘게 나눠 옮겨요.
// 걸음마다 "여기 벽이야?"를 확인하니까, 빨리 달려도 벽을 뚫고 지나가지 않아요.

#include "game/physics/tile_collision.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wind::game {
namespace {

using engine::RectF;
using engine::Vec2;

// 한 번에 옮기는 최대 거리 (픽셀). 이보다 작게 나눠서 옮겨요.
constexpr float kMaxSubstep = 1.0f;
// 네모 오른쪽·아래쪽 끝을 칸 번호로 바꿀 때 살짝 빼는 값.
// 상자가 칸 경계에 "딱 닿기만" 했을 때 옆 칸까지 걸친 것으로 치지 않으려고요.
constexpr float kEdgeEpsilon = 0.001f;

// 상자를 (dx, dy)만큼 옮겨 봤을 때 막히나요?
bool blocked_at(const world::MapData& map, const RectF& box, float dx, float dy) {
    return box_blocked(map, {box.x + dx, box.y + dy, box.w, box.h});
}

// 한 축으로 step만큼 움직이려다 막혔을 때, 다른 축으로 살짝 비켜서 지나갈 수 있나 찾아요.
// 가까운 쪽(1픽셀, 2픽셀...)부터 양쪽을 다 봐요. 찾으면 비켜야 할 방향(-1 또는 +1), 못 찾으면 0.
int find_corner_slip(const world::MapData& map, const RectF& box, bool moving_x, float step,
                     float corner_assist) {
    const int max_offset = static_cast<int>(corner_assist);
    for (int offset = 1; offset <= max_offset; ++offset) {
        for (const int sign : {-1, 1}) {
            const float side = static_cast<float>(sign * offset);
            const float dx = moving_x ? step : side;
            const float dy = moving_x ? side : step;
            if (!blocked_at(map, box, dx, dy)) {
                return sign;
            }
        }
    }
    return 0;
}

}  // namespace

RectF body_box(Vec2 feet, const BodyShape& shape) {
    return {feet.x - shape.width * 0.5f, feet.y - shape.height, shape.width, shape.height};
}

bool box_blocked(const world::MapData& map, const RectF& box) {
    if (map.tile_size <= 0) {
        return true;  // 잘못된 맵이면 아무 데도 못 가요
    }
    const auto size = static_cast<float>(map.tile_size);
    // 상자가 걸친 칸 범위를 구해요. (음수 좌표도 맞게 내림)
    const auto left = static_cast<std::int32_t>(std::floor(box.x / size));
    const auto top = static_cast<std::int32_t>(std::floor(box.y / size));
    const auto right = static_cast<std::int32_t>(std::floor((box.right() - kEdgeEpsilon) / size));
    const auto bottom = static_cast<std::int32_t>(std::floor((box.bottom() - kEdgeEpsilon) / size));
    for (std::int32_t y = top; y <= bottom; ++y) {
        for (std::int32_t x = left; x <= right; ++x) {
            // is_walkable은 맵 밖이면 false라서, 맵 밖도 벽처럼 막혀요.
            if (!map.is_walkable({x, y})) {
                return true;
            }
        }
    }
    return false;
}

MoveResult move_and_slide(const world::MapData& map, const BodyShape& shape, Vec2 feet, Vec2 delta,
                          float corner_assist) {
    MoveResult result{feet};
    // 몇 번에 나눠 옮길지: 더 많이 움직이는 축을 기준으로 1픽셀 이하가 되게
    const float longest = std::max(std::abs(delta.x), std::abs(delta.y));
    const int steps = std::max(1, static_cast<int>(std::ceil(longest / kMaxSubstep)));
    const Vec2 step{delta.x / static_cast<float>(steps), delta.y / static_cast<float>(steps)};
    // 한 축으로만 움직이려 할 때만 모서리 보정을 해요.
    // (대각선으로 갈 때는 벽을 따라 미끄러지기만으로 충분해요)
    const bool pure_x = step.y == 0.0f;
    const bool pure_y = step.x == 0.0f;

    for (int i = 0; i < steps; ++i) {
        // 1) 가로로 한 걸음
        if (step.x != 0.0f && !result.hit_x) {
            const RectF box = body_box(result.feet, shape);
            if (!blocked_at(map, box, step.x, 0.0f)) {
                result.feet.x += step.x;
            } 
            else if (const int slip = pure_x ? find_corner_slip(map, box, true, step.x, corner_assist)
                                               : 0;
                       slip != 0) {
                // 모서리에 걸렸어요: 세로로 살짝 비켜요 (한 걸음 크기만큼만, 그래서 부드러워요)
                const float nudge = static_cast<float>(slip) * std::abs(step.x);
                if (!blocked_at(map, box, 0.0f, nudge)) {
                    result.feet.y += nudge;
                }
            } else {
                result.hit_x = true;  // 진짜 벽이에요. 가로로는 더 못 가요.
            }
        }
        // 2) 세로로 한 걸음
        if (step.y != 0.0f && !result.hit_y) {
            const RectF box = body_box(result.feet, shape);
            if (!blocked_at(map, box, 0.0f, step.y)) {
                result.feet.y += step.y;
            } else if (const int slip = pure_y ? find_corner_slip(map, box, false, step.y, corner_assist)
                                               : 0;
                       slip != 0) {
                const float nudge = static_cast<float>(slip) * std::abs(step.y);
                if (!blocked_at(map, box, nudge, 0.0f)) {
                    result.feet.x += nudge;
                }
            } else {
                result.hit_y = true;
            }
        }
    }
    return result;
}

}  // namespace wind::game
