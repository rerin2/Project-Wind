// 이 파일은 무엇을 하나요?
// 필드 싸움 담당(FieldCombat)을 저장소의 진짜 롱소드 데이터로 숙제 검사해요. (9단계 완료 조건)
// - 허수아비 앞에서 공격 버튼을 계속 누르면 콤보 3타가 이어지고, HP가 14 -> 15 -> 21만큼 줄어요
// - 칼이 안 닿는 곳에서 휘두르면 HP가 그대로예요
#include "game/states/field_combat.hpp"

#include "wind/content/weapon_loader.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace {

using wind::game::FieldCombat;
using wind::game::PlayerMotion;
using wind::game::TrainingDummy;

constexpr double kDt = 1.0 / 60.0;

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

PlayerMotion make_player() {
    auto weapon = wind::content::load_weapon_file(WIND_TEST_CONTENT_DIR, "weapons/longsword.json");
    EXPECT_TRUE(weapon.ok());
    return PlayerMotion({100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15, std::move(*weapon.value));
}

TrainingDummy make_dummy(float feet_x) {
    wind::combat::TrainingDummyData data;
    data.max_hp = 100;
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    return TrainingDummy(1, "d", data, {feet_x, 300.0f});
}

// steps 박자 동안 공격 버튼을 계속 누르며 맞은 양을 모아요.
std::vector<int> swing_for(PlayerMotion& player, FieldCombat& combat, int steps) {
    const auto map = open_map();
    std::vector<int> amounts;
    for (int i = 0; i < steps; ++i) {
        player.press_attack();
        player.step(kDt, {}, false, map);
        combat.step(kDt, player);
        for (const auto& hit : combat.hits()) amounts.push_back(hit.amount);
    }
    return amounts;
}

TEST(FieldCombat, LongswordComboHitsDummyThreeTimes) {
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)});    // 오른쪽 바로 앞 (칼이 닿는 곳)
    const auto amounts = swing_for(player, combat, 60);  // 1초 동안 계속 누르기
    ASSERT_GE(amounts.size(), 3u);
    EXPECT_EQ(amounts[0], 14);  // 1타: 14 × 1.0
    EXPECT_EQ(amounts[1], 15);  // 2타: 14 × 1.1 = 15.4
    EXPECT_EQ(amounts[2], 21);  // 3타: 14 × 1.5
    EXPECT_EQ(combat.dummies()[0].hp(), 100 - 14 - 15 - 21 - (amounts.size() > 3 ? amounts[3] : 0));
    ASSERT_TRUE(combat.last_hit().has_value());
}

TEST(FieldCombat, SwingOutOfReachDoesNothing) {
    auto player = make_player();
    FieldCombat combat({make_dummy(260.0f)});    // 멀리 있어요
    const auto amounts = swing_for(player, combat, 30);
    EXPECT_TRUE(amounts.empty());
    EXPECT_EQ(combat.dummies()[0].hp(), 100);
    EXPECT_FALSE(combat.last_hit().has_value());
    EXPECT_EQ(combat.debug_text(), "dummies 1");  // 아직 아무도 안 맞았어요
}

// F3 정보판 싸움 줄: 마지막으로 맞힌 피해, 대상 번호, 남은 HP
TEST(FieldCombat, DebugTextShowsLastHit) {
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)});
    (void)swing_for(player, combat, 12);           // 1타만 맞을 만큼
    EXPECT_EQ(combat.debug_text(), "dummies 1  last hit 14 -> #1 86/100");
}

}  // namespace
