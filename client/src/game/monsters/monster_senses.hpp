#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터가 주인공을 "보는" 규칙을 모은 도우미 함수들이에요. (명세 9.6, 11b. 13c에서 Monster에서 옮겨 왔어요)
// 모두 거리만 보는 순수 함수라 같은 값이면 늘 같은 답이에요. 값의 이유는 docs/GAMEPLAY.md "Fighting" 절에 있어요.

#include "engine/core/math.hpp"

#include "wind/combat/monster_data.hpp"

#include <optional>

namespace wind::game {

// 쉬거나 걷다가 알아채나요? 주인공이 감지 거리 안이면 알아채요.
[[nodiscard]] bool monster_notices(const combat::MonsterData& data, engine::Vec2 feet,
                                   std::optional<engine::Vec2> hero);
// 돌아가는 중에 다시 알아채나요? 알아챌 거리 안이면서, 주인공이 영역(제자리에서 추적 한계 − 공격 거리) 안일 때만.
// 경계선에서 알아챘다 포기했다를 되풀이하지 않게 여유를 둬요.
[[nodiscard]] bool monster_notices_while_returning(const combat::MonsterData& data, engine::Vec2 feet,
                                                  engine::Vec2 home, std::optional<engine::Vec2> hero);
// 계속 쫓을 수 있나요? 몬스터가 제자리에서, 주인공이 몬스터에게서 추적 한계보다 멀어지지 않았으면 돼요.
[[nodiscard]] bool monster_can_keep_chasing(const combat::MonsterData& data, engine::Vec2 feet, engine::Vec2 home,
                                            std::optional<engine::Vec2> hero);

}  // namespace wind::game
