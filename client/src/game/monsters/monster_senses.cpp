// 이 파일은 무엇을 하나요?
// 몬스터가 주인공을 알아채고, 다시 알아채고, 계속 쫓을지 정하는 거리 규칙.

#include "game/monsters/monster_senses.hpp"

namespace wind::game {
namespace {

// 두 점 사이 거리
float distance(engine::Vec2 a, engine::Vec2 b) { return engine::length(a - b); }

}  // namespace

bool monster_notices(const combat::MonsterData& data, engine::Vec2 feet, std::optional<engine::Vec2> hero) {
    return hero.has_value() && distance(*hero, feet) <= data.detect_range;
}

bool monster_notices_while_returning(const combat::MonsterData& data, engine::Vec2 feet, engine::Vec2 home,
                                     std::optional<engine::Vec2> hero) {
    const float territory = data.leash_range - data.attack_range;  // 영역 (추적 한계 − 공격 거리)
    return monster_notices(data, feet, hero) && distance(*hero, home) <= territory;
}

bool monster_can_keep_chasing(const combat::MonsterData& data, engine::Vec2 feet, engine::Vec2 home,
                              std::optional<engine::Vec2> hero) {
    return hero.has_value() && distance(feet, home) <= data.leash_range && distance(*hero, feet) <= data.leash_range;
}

}  // namespace wind::game
