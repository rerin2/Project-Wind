// 이 파일은 무엇을 하나요?
// 드롭과 자동 줍기 규칙을 숙제 검사(테스트)해요. (명세 9.7, 10.2, 14b)
// - 몬스터가 쓰러지면 드롭이 바닥에 생기고, 가까이 있으면 가방으로 들어와요
// - 같은 드롭은 한 번만 주워져요
// - 떨어지는 동안(0.6초)은 못 주워요
// - 가방이 가득 차면 바닥에 남고, 알림은 한 번만 (멀어졌다 오면 다시)
#include "game/items/field_loot.hpp"

#include "game/items/map_loot.hpp"
#include "game/states/field_combat.hpp"

#include "wind/content/monster_loader.hpp"
#include "wind/content/weapon_loader.hpp"
#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::FieldCombat;
using wind::game::FieldLoot;
using wind::game::MapLoot;
using wind::game::PlayerMotion;

constexpr double kDt = 1.0 / 60.0;

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

PlayerMotion make_player(float x) {
    auto weapon = wind::content::load_weapon_file(WIND_TEST_CONTENT_DIR, "weapons/longsword.json");
    EXPECT_TRUE(weapon.ok());
    return PlayerMotion({x, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15, std::move(*weapon.value));
}

// 젤리 하나뿐인 도감 (99개씩 쌓여요)
wind::items::ItemCatalog make_catalog() {
    wind::items::ItemCatalog catalog;
    wind::items::ItemDef jelly;
    jelly.id = "jelly";
    jelly.max_stack = 99;
    catalog.add(jelly);
    return catalog;
}

// 진짜 슬라임인데 드롭 표만 "젤리 2개, 100%"로 바꿔요 (결과를 확실하게)
wind::game::Monster make_slime(float feet_x) {
    auto data = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    EXPECT_TRUE(data.ok());
    data.value->drops = {{"jelly", 1.0f, 2, 2}};
    return wind::game::Monster("slime", std::move(*data.value), {feet_x, 300.0f});
}

TEST(FieldLoot, SlainMonsterDropsAndHeroPicksUpExactlyOnce) {
    auto player = make_player(100.0f);
    FieldCombat combat({}, 1, 0.0f, {make_slime(125.0f)});  // 칼이 닿는 곳
    const auto map = open_map();
    const auto catalog = make_catalog();
    wind::items::Inventory bag;
    MapLoot loot;
    FieldLoot field(loot, 3);
    int pickups = 0;
    std::size_t most_on_ground = 0;
    for (int i = 0; i < 360; ++i) {  // 쓰러뜨리고, 드롭이 떨어지고, 걸어가서 주울 때까지 (6초)
        wind::engine::Vec2 walk{};  // 쓰러뜨릴 때까지는 제자리에서 칼질
        if (!combat.monsters()[0].defeated()) player.press_attack();
        else if (!loot.items().empty()) walk = wind::engine::normalized(loot.items()[0].at - player.mover().feet());
        player.step(kDt, walk, false, map);
        combat.step(kDt, player, map);
        field.step(static_cast<float>(kDt), combat, player, map, bag, catalog);
        pickups += static_cast<int>(field.picked().size());
        most_on_ground = std::max(most_on_ground, loot.items().size());
    }
    EXPECT_TRUE(combat.monsters()[0].defeated());
    EXPECT_EQ(most_on_ground, 1u);           // 바닥에 한 묶음이 생겼다가
    EXPECT_EQ(pickups, 1);                   // 딱 한 번 주워지고
    EXPECT_EQ(bag.count_of("jelly"), 2);     // 가방에 2개
    EXPECT_TRUE(loot.items().empty());       // 바닥에서는 사라져요
}

TEST(FieldLoot, CannotPickUpWhileItIsStillBouncing) {
    const auto player = make_player(100.0f);
    const FieldCombat combat({}, 1, 0.0f);
    const auto map = open_map();
    const auto catalog = make_catalog();
    wind::items::Inventory bag;
    MapLoot loot;
    loot.drop({"jelly", 1}, {100.0f, 300.0f}, {110.0f, 300.0f});  // 주인공 바로 옆
    FieldLoot field(loot, 3);
    for (int i = 0; i < 30; ++i) field.step(static_cast<float>(kDt), combat, player, map, bag, catalog);  // 0.5초
    EXPECT_EQ(bag.count_of("jelly"), 0);  // 아직 튀는 중
    for (int i = 0; i < 12; ++i) field.step(static_cast<float>(kDt), combat, player, map, bag, catalog);  // 0.7초
    EXPECT_EQ(bag.count_of("jelly"), 1);
}

TEST(FieldLoot, FarItemsStayOnTheGround) {
    const auto player = make_player(100.0f);
    const FieldCombat combat({}, 1, 0.0f);
    const auto map = open_map();
    const auto catalog = make_catalog();
    wind::items::Inventory bag;
    MapLoot loot;
    loot.drop({"jelly", 1}, {}, {100.0f + FieldLoot::kPickupRadius + 2.0f, 300.0f});  // 주울 거리 바로 밖
    FieldLoot field(loot, 3);
    for (int i = 0; i < 60; ++i) field.step(static_cast<float>(kDt), combat, player, map, bag, catalog);
    EXPECT_EQ(loot.items().size(), 1u);
    EXPECT_EQ(bag.count_of("jelly"), 0);
}

TEST(FieldLoot, FullBagLeavesItemAndWarnsOnceUntilHeroWalksAway) {
    const auto near = make_player(100.0f);
    const auto far = make_player(400.0f);  // 48픽셀보다 훨씬 멀리
    const FieldCombat combat({}, 1, 0.0f);
    const auto map = open_map();
    const auto catalog = make_catalog();
    wind::items::Inventory bag(0);  // 칸이 없는 가방 = 늘 가득 참
    MapLoot loot;
    loot.drop({"jelly", 3}, {}, {110.0f, 300.0f});
    FieldLoot field(loot, 3);
    int warnings = 0;
    for (int i = 0; i < 120; ++i) {  // 2초 동안 옆에 서 있어도
        field.step(static_cast<float>(kDt), combat, near, map, bag, catalog);
        warnings += static_cast<int>(field.refused().size());
    }
    EXPECT_EQ(warnings, 1);               // 알림은 한 번
    EXPECT_EQ(loot.items().size(), 1u);   // 바닥에 그대로
    EXPECT_EQ(loot.items()[0].stack.count, 3);
    field.step(static_cast<float>(kDt), combat, far, map, bag, catalog);   // 멀어졌다가
    field.step(static_cast<float>(kDt), combat, near, map, bag, catalog);  // 다시 오면
    EXPECT_EQ(field.refused().size(), 1u);  // 또 알려요
}

TEST(FieldLoot, DropsLandAroundTheMonsterOnWalkableTiles) {
    auto player = make_player(100.0f);
    FieldCombat combat({}, 1, 0.0f, {make_slime(125.0f)});
    const auto map = open_map();
    const auto catalog = make_catalog();
    wind::items::Inventory bag(0);  // 못 줍게 해서 바닥에 남겨요
    MapLoot loot;
    FieldLoot field(loot, 9);
    for (int i = 0; i < 240 && loot.items().empty(); ++i) {
        player.press_attack();
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        field.step(static_cast<float>(kDt), combat, player, map, bag, catalog);
    }
    ASSERT_EQ(loot.items().size(), 1u);
    const auto& item = loot.items()[0];
    const float distance = wind::engine::length(item.at - item.from);
    EXPECT_GE(distance, 9.9f);   // 쓰러진 자리에서 10~20픽셀
    EXPECT_LE(distance, 20.1f);
}

}  // namespace

// ---- 14c: 인벤토리에서 발 앞에 내려놓기 ----

namespace {

TEST(FieldLoot, PutDownMovesWholeStackInFrontAndWaitsUntilHeroLeaves) {
    const auto near = make_player(100.0f);  // 오른쪽을 봐요
    const auto far = make_player(400.0f);
    const FieldCombat combat({}, 1, 0.0f);
    const auto map = open_map();
    const auto catalog = make_catalog();
    wind::items::Inventory bag;
    ASSERT_TRUE(bag.try_add(catalog, "jelly", 5));
    MapLoot loot;
    FieldLoot field(loot, 3);
    ASSERT_TRUE(field.put_down(0, near, map, bag));
    EXPECT_EQ(bag.count_of("jelly"), 0);  // 묶음 전부 나왔어요
    ASSERT_EQ(loot.items().size(), 1u);
    EXPECT_EQ(loot.items()[0].stack.count, 5);
    EXPECT_FLOAT_EQ(loot.items()[0].at.x, 114.0f);  // 바라보는 쪽 14픽셀 앞
    for (int i = 0; i < 120; ++i) field.step(static_cast<float>(kDt), combat, near, map, bag, catalog);
    EXPECT_EQ(bag.count_of("jelly"), 0);  // 옆에 있어도 바로 다시 빨려 들지 않아요
    field.step(static_cast<float>(kDt), combat, far, map, bag, catalog);   // 멀어졌다가
    field.step(static_cast<float>(kDt), combat, near, map, bag, catalog);  // 돌아오면
    EXPECT_EQ(bag.count_of("jelly"), 5);  // 다시 주워요
    EXPECT_TRUE(loot.items().empty());
}

TEST(FieldLoot, PutDownEmptySlotChangesNothing) {
    const auto hero = make_player(100.0f);
    const auto map = open_map();
    wind::items::Inventory bag;
    MapLoot loot;
    FieldLoot field(loot, 3);
    EXPECT_FALSE(field.put_down(0, hero, map, bag));
    EXPECT_FALSE(field.put_down(99, hero, map, bag));  // 밖의 칸도 멈추지 않아요
    EXPECT_TRUE(loot.items().empty());
}

TEST(FieldLoot, PutDownNextToWallLandsAtFeet) {
    const auto hero = make_player(90.0f);  // 발은 (90, 300) = 칸 (2, 9), 오른쪽을 봐요
    auto map = open_map();
    map.walkable[static_cast<std::size_t>(9 * map.width + 3)] = 0;  // 발 앞(104, 300) = 칸 (3, 9)이 벽
    const auto catalog = make_catalog();
    wind::items::Inventory bag;
    ASSERT_TRUE(bag.try_add(catalog, "jelly", 1));
    MapLoot loot;
    FieldLoot field(loot, 3);
    ASSERT_TRUE(field.put_down(0, hero, map, bag));
    EXPECT_FLOAT_EQ(loot.items()[0].at.x, 90.0f);  // 벽 안에 놓지 않아요
}

}  // namespace
