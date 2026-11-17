// 이 파일은 무엇을 하나요?
// 몬스터 한 박자 걷기(목표 점으로, 정해진 방향으로)를 계산하는 곳이에요.

#include "game/monsters/monster_moves.hpp"

#include <algorithm>

namespace wind::game {
namespace {

// 한 박자에 가야 할 거리의 절반도 못 가면 "막힘"이에요
constexpr float kStuckRatio = 0.5f;
// 벽 모서리 보정 (픽셀). 주인공보다 작게: 몬스터는 길을 정확히 몰라도 돼요.
constexpr float kCornerAssist = 2.0f;

// delta만큼 옮기고, 얼마나 갔는지와 막혔는지를 적어요
MonsterStep slide(const world::MapData& map, const BodyShape& body, engine::Vec2 feet, engine::Vec2 delta,
                  float expected) {
    const MoveResult moved = move_and_slide(map, body, feet, delta, kCornerAssist);  // 벽 미끄러짐
    MonsterStep step;
    step.feet = moved.feet;
    step.travelled = engine::length(moved.feet - feet);              // 실제로 간 거리
    step.blocked = step.travelled < expected * kStuckRatio;          // 절반도 못 갔으면 막힘
    step.arrived = !moved.hit_x && !moved.hit_y;                     // 부르는 쪽이 "마지막 걸음"일 때만 써요
    return step;
}

}  // namespace

MonsterStep step_toward(const world::MapData& map, const BodyShape& body, engine::Vec2 feet, engine::Vec2 target,
                        float speed, float dt_seconds) {
    const engine::Vec2 to_target = target - feet;                     // 남은 길
    const float remaining = engine::length(to_target);                // 남은 거리
    const float stride = speed * dt_seconds;                          // 이번 박자에 갈 거리
    const bool last_step = remaining <= stride;                       // 이번에 도착하나
    const engine::Vec2 delta = last_step ? to_target : engine::normalized(to_target) * stride;
    MonsterStep step = slide(map, body, feet, delta, std::min(stride, remaining));
    step.arrived = last_step && step.arrived;                         // 막힘 없이 마지막 걸음을 다 갔을 때만 도착
    return step;
}

MonsterStep step_along(const world::MapData& map, const BodyShape& body, engine::Vec2 feet, engine::Vec2 direction,
                       float distance) {
    MonsterStep step = slide(map, body, feet, direction * distance, distance);
    step.arrived = false;  // 덮치기에는 목표 점이 없어요
    return step;
}

}  // namespace wind::game
