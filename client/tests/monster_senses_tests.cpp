// 이 파일은 무엇을 하나요?
// 몬스터 거리 규칙 검사 (13c에서 Monster에서 옮긴 함수): 알아채기, 돌아가며 다시 알아채기, 계속 쫓기.
#include "game/monsters/monster_senses.hpp"

#include <gtest/gtest.h>

namespace {

wind::combat::MonsterData data() {
    wind::combat::MonsterData d;
    d.detect_range = 120.0f;
    d.attack_range = 40.0f;
    d.leash_range = 260.0f;
    return d;
}

}  // namespace

TEST(MonsterSenses, NoticesInsideDetectRange) {
    EXPECT_TRUE(wind::game::monster_notices(data(), {0.0f, 0.0f}, wind::engine::Vec2{120.0f, 0.0f}));
    EXPECT_FALSE(wind::game::monster_notices(data(), {0.0f, 0.0f}, wind::engine::Vec2{121.0f, 0.0f}));
    EXPECT_FALSE(wind::game::monster_notices(data(), {0.0f, 0.0f}, std::nullopt));  // 쓰러진 주인공
}

TEST(MonsterSenses, ReturningNoticesOnlyInsideTerritory) {
    // 영역 = 260 − 40 = 220. 몬스터는 제자리에서 200 떨어져 돌아가는 중
    const wind::engine::Vec2 home{0.0f, 0.0f};
    const wind::engine::Vec2 feet{200.0f, 0.0f};
    EXPECT_TRUE(wind::game::monster_notices_while_returning(data(), feet, home, wind::engine::Vec2{210.0f, 0.0f}));
    EXPECT_FALSE(wind::game::monster_notices_while_returning(data(), feet, home, wind::engine::Vec2{230.0f, 0.0f}));
}

TEST(MonsterSenses, LeashLimitsChase) {
    const wind::engine::Vec2 home{0.0f, 0.0f};
    EXPECT_TRUE(wind::game::monster_can_keep_chasing(data(), {100.0f, 0.0f}, home, wind::engine::Vec2{300.0f, 0.0f}));
    EXPECT_FALSE(wind::game::monster_can_keep_chasing(data(), {270.0f, 0.0f}, home, wind::engine::Vec2{280.0f, 0.0f}));
}
