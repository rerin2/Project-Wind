#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터가 한 박자 걸어가는 계산을 모은 도우미 함수예요. (11b, monster.cpp에서 나눔)
// - step_toward: 목표 점으로 정해진 빠르기로 걸어가요 (배회, 쫓기, 복귀)
// - step_along:  정해진 방향으로 정해진 거리만큼 나가요 (덮치기)
// 둘 다 벽에 닿으면 주인공과 같은 move_and_slide로 벽을 따라 미끄러져요.
// 게임 규칙이라 SDL을 몰라요.

#include "engine/core/math.hpp"
#include "game/physics/tile_collision.hpp"

#include "wind/world/map.hpp"

namespace wind::game {

// 한 박자 걸은 결과
struct MonsterStep {
    engine::Vec2 feet;       // 걸은 뒤 발 위치
    float travelled = 0.0f;  // 실제로 간 거리 (픽셀)
    bool arrived = false;    // 막힘 없이 목표에 닿았나 (step_toward만)
    bool blocked = false;    // 가야 할 거리의 절반도 못 갔나 (벽에 막힘)
};

// feet에서 target으로 speed(픽셀/초)로 dt초 걸어가요. 목표를 지나치지 않아요.
[[nodiscard]] MonsterStep step_toward(const world::MapData& map, const BodyShape& body, engine::Vec2 feet,
                                      engine::Vec2 target, float speed, float dt_seconds);

// feet에서 direction(길이 1) 쪽으로 distance만큼 나가요.
[[nodiscard]] MonsterStep step_along(const world::MapData& map, const BodyShape& body, engine::Vec2 feet,
                                     engine::Vec2 direction, float distance);

}  // namespace wind::game
