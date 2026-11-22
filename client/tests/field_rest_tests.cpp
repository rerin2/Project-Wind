// 이 파일은 무엇을 하나요?
// 필드 쿠션 담당 검사 (13b, 명세 19장 단계 13 완료 조건)
// - 쿠션 가까이에서 E: 앉고, HP·MP·스태미나가 가득 차고, 잠깐 뒤 쉬기 이동 요청이 한 번 나와요
// - 몬스터에게 쫓기는 중에는 쉴 수 없고 "못 쉼" 횟수가 올라요
// - 앉아 있을 때 E·방향키: 일어나요
// - 새 맵에서 쿠션 위에 앉은 채 시작하면 쉬기 이동을 다시 요청하지 않아요
#include "game/states/field_rest.hpp"

#include "game/entities/player_motion.hpp"
#include "game/states/field_combat.hpp"

#include "wind/content/monster_loader.hpp"

#include <gtest/gtest.h>

using wind::engine::Vec2;
using wind::game::Cushion;
using wind::game::FieldCombat;
using wind::game::FieldRest;
using wind::game::PlayerMotion;

namespace {

constexpr double kDt = 1.0 / 60.0;
const Vec2 kSeat{112.0f, 128.0f};  // 쿠션 자리 (칸 3,3)

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 40;
    map.height = 20;
    map.walkable.assign(static_cast<std::size_t>(40 * 20), 1);
    return map;
}

FieldRest make_rest() {
    return FieldRest({Cushion{"test_cushion", "시험 쿠션", {3, 3}, kSeat}});
}

PlayerMotion make_player(Vec2 feet) {
    return PlayerMotion(feet, wind::world::Direction::Right, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15);
}

// 한 박자: 주인공 -> 싸움 -> 쿠션 (필드와 같은 순서)
void tick(PlayerMotion& player, FieldCombat& combat, FieldRest& rest, const wind::world::MapData& map,
          Vec2 move = {}) {
    player.step(kDt, move, false, map);
    combat.step(kDt, player, map);
    rest.step(static_cast<float>(kDt), player, combat);
}

}  // namespace

TEST(FieldRest, RestingRefillsAndRequestsTravelOnce) {
    const auto map = open_map();
    auto player = make_player({120.0f, 130.0f});  // 쿠션에서 약 8픽셀
    FieldCombat combat({});
    auto rest = make_rest();
    player.take_hit(30, {1.0f, 0.0f}, 0.0f);      // HP를 깎아 둬요 (넉백 0)
    rest.press_interact();                         // 방금 맞았으니 못 쉬어요 (3초 규칙)
    tick(player, combat, rest, map);
    EXPECT_EQ(rest.denied_count(), 1u);
    for (int i = 0; i < 200; ++i) tick(player, combat, rest, map);  // 3초 넘게 기다리면 조용해져요
    ASSERT_LT(player.health().current(), player.health().max());
    ASSERT_NE(rest.prompt_cushion(player), nullptr);  // "[E] 쉬기" 안내가 보여요

    rest.press_interact();
    tick(player, combat, rest, map);
    EXPECT_EQ(player.state(), wind::game::CharacterState::Resting);
    EXPECT_EQ(player.health().current(), player.health().max());  // 바로 가득 (명세 8.7 쉬기 2번)
    EXPECT_FLOAT_EQ(player.stamina().current(), player.stamina().max());
    EXPECT_FALSE(player.can_be_hit());                            // 쉬는 동안은 안 맞아요
    EXPECT_EQ(rest.prompt_cushion(player), nullptr);              // 이미 쉬니까 안내는 사라져요

    int requests = 0;
    for (int i = 0; i < 60; ++i) {  // 1초
        tick(player, combat, rest, map);
        if (auto request = rest.take_travel_request()) {
            ++requests;
            EXPECT_EQ(*request, "test_cushion");
        }
    }
    EXPECT_EQ(requests, 1);                                       // 앉고 0.6초 뒤 한 번
    EXPECT_NEAR(player.mover().feet().x, kSeat.x, 0.01f);         // 쿠션 가운데에 앉았어요
}

TEST(FieldRest, CannotRestWhileSlimeChases) {
    auto data = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    ASSERT_TRUE(data.ok());
    const auto map = open_map();
    auto player = make_player({112.0f, 128.0f});
    FieldCombat combat({}, 1, 0.0f, {wind::game::Monster("slime", *data.value, {180.0f, 128.0f})});  // 68픽셀 옆
    auto rest = make_rest();
    for (int i = 0; i < 40 && !combat.monster_engaged(); ++i) tick(player, combat, rest, map);
    ASSERT_TRUE(combat.monster_engaged());   // 슬라임이 알아챘어요
    rest.press_interact();
    tick(player, combat, rest, map);
    EXPECT_NE(player.state(), wind::game::CharacterState::Resting);  // 못 쉬어요
    EXPECT_EQ(rest.denied_count(), 1u);      // "주변이 너무 소란스럽다"
}

TEST(FieldRest, InteractOrMoveStandsUp) {
    const auto map = open_map();
    auto player = make_player(kSeat);
    FieldCombat combat({});
    auto rest = make_rest();
    ASSERT_TRUE(rest.sit_on("test_cushion", player));   // 새 맵에서 앉은 채 시작
    for (int i = 0; i < 60; ++i) tick(player, combat, rest, map);
    EXPECT_FALSE(rest.take_travel_request().has_value());  // 이미 쉬었으니 다시 요청하지 않아요
    ASSERT_EQ(player.rest().phase(), wind::game::HeroRest::Phase::Sitting);

    rest.press_interact();                                 // E
    tick(player, combat, rest, map);
    EXPECT_EQ(player.rest().phase(), wind::game::HeroRest::Phase::Standing);
    for (int i = 0; i < 40; ++i) tick(player, combat, rest, map);  // 기지개가 끝나면
    EXPECT_EQ(player.state(), wind::game::CharacterState::Idle);

    ASSERT_TRUE(rest.sit_on("test_cushion", player));
    tick(player, combat, rest, map, {0.0f, 1.0f});          // 방향키로도 일어나요
    EXPECT_EQ(player.rest().phase(), wind::game::HeroRest::Phase::Standing);
}

TEST(FieldRest, FarFromCushionDoesNothing) {
    const auto map = open_map();
    auto player = make_player({400.0f, 128.0f});
    FieldCombat combat({});
    auto rest = make_rest();
    rest.press_interact();
    tick(player, combat, rest, map);
    EXPECT_FALSE(player.rest().active());
    EXPECT_EQ(rest.denied_count(), 0u);
    EXPECT_FALSE(rest.sit_on("no_such_cushion", player));
}
