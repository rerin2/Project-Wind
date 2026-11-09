#pragma once

// 이 파일은 무엇을 하나요?
// 캐릭터의 "발 상자"가 벽(못 가는 칸)에 부딪히는지 검사하고,
// 부딪히면 벽을 따라 미끄러지게 옮겨 주는 도우미예요. (명세 7.2)
//
// 발 상자 = 캐릭터 발밑의 작은 네모예요. 그림 전체가 아니라 발만 벽에 부딪혀요.
// 그래서 머리가 나무 뒤로 살짝 겹쳐 보여도 괜찮아요. (위에서 내려다보는 게임이라서요)
//
// 규칙
// - 가로(x)와 세로(y)를 따로따로 움직여요. 대각선으로 벽에 부딪히면
//   막힌 쪽만 멈추고 나머지 쪽으로는 계속 가요 = 벽을 따라 미끄러지기.
// - 맵 밖은 벽과 똑같이 막혀 있어요.
// - 모서리 보정: 벽 모서리에 몇 픽셀만 걸렸다면 살짝 옆으로 밀어서 지나가게 해 줘요.
//   좁은 길에 들어갈 때 픽셀 하나 때문에 막히는 답답함을 없애 줘요.

#include "engine/core/math.hpp"

#include "wind/world/map.hpp"

namespace wind::game {

// 발 상자의 크기. 발 위치(feet)는 상자 아래쪽 가운데예요.
struct BodyShape {
    float width = 14.0f;
    float height = 8.0f;
};

// 발 위치와 상자 크기로 맵 위의 네모를 만들어요.
[[nodiscard]] engine::RectF body_box(engine::Vec2 feet, const BodyShape& shape);

// 이 네모가 벽이나 맵 밖에 조금이라도 걸치나요?
[[nodiscard]] bool box_blocked(const world::MapData& map, const engine::RectF& box);

// 옮긴 결과
struct MoveResult {
    engine::Vec2 feet;   // 옮긴 뒤 발 위치
    bool hit_x = false;  // 가로로 가다가 벽에 막혔나
    bool hit_y = false;  // 세로로 가다가 벽에 막혔나
};

// 발 위치를 delta만큼 옮겨요. 벽에 닿으면 그 축은 멈추고, 다른 축은 계속 가요.
// corner_assist: 모서리를 몇 픽셀까지 봐줄지 (0이면 보정 안 함)
[[nodiscard]] MoveResult move_and_slide(const world::MapData& map, const BodyShape& shape,
                                        engine::Vec2 feet, engine::Vec2 delta, float corner_assist);

}  // namespace wind::game
