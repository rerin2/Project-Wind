// 이 파일은 무엇을 하나요?
// 필드 아이템 담당의 물약 쓰기 요청(전부 성공 또는 전부 실패, 알림)을 숙제 검사(테스트)해요. (14d)
// 드롭·줍기·내려놓기 규칙은 field_loot_tests가 봐요.
#include "game/states/field_items.hpp"

#include "game/items/loot_store.hpp"
#include "game/states/field_combat.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/items/quick_slots.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Vec2;
using wind::game::LootNotice;
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

// 회복 물약(소비)과 젤리(재료) 도감
wind::items::ItemCatalog make_catalog() {
    wind::items::ItemCatalog catalog;
    wind::items::ItemDef potion;
    potion.id = "potion";
    potion.name = "회복 물약";
    potion.max_stack = 20;
    potion.kind = wind::items::ItemKind::Consumable;
    potion.use = wind::items::UseEffect{50, 0};
    catalog.add(potion);
    wind::items::ItemDef jelly;
    jelly.id = "jelly";
    jelly.max_stack = 99;
    catalog.add(jelly);
    return catalog;
}

// 필드 아이템 담당 하나와 그것이 빌려 쓰는 것들 (그림은 없어도 돼요)
struct Fixture {
    wind::items::ItemCatalog catalog = make_catalog();
    wind::items::Inventory bag;
    wind::game::LootStore store;
    wind::game::FieldItems items{wind::game::ItemWorld{bag, catalog, store}, store.on("maps/test.json"), {}, 1};
    wind::world::MapData map = open_map();
    wind::game::FieldCombat combat{{}, 1, 0.0f};
    PlayerMotion hero{{100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                      wind::game::StaminaTuning{}, 0.15};

    Fixture() { bag.try_add(catalog, "potion", 3); }  // 0번 칸: 물약 3개
    void hurt() {
        (void)hero.take_hit(30, {1.0f, 0.0f}, 0.0f);
        run(90);  // 경직이 풀릴 때까지
    }
    void run(int steps) {
        for (int i = 0; i < steps; ++i) {
            hero.step(kDt, {}, false, map);
            items.step(static_cast<float>(kDt), combat, hero, map);
        }
    }
};

TEST(FieldItems, UsingAPotionTakesOneAndHeals) {
    Fixture f;
    f.hurt();
    f.items.request_use(0);
    f.run(1);
    EXPECT_EQ(f.bag.count_of("potion"), 2);  // 시작할 때 하나를 써요
    EXPECT_TRUE(f.items.take_notices().empty());
    f.run(30);  // 꿀꺽 지나서
    EXPECT_EQ(f.hero.health().current(), 100);
}

TEST(FieldItems, FullHealthRefusesAndKeepsThePotion) {
    Fixture f;
    f.items.request_use(0);
    f.run(1);
    EXPECT_EQ(f.bag.count_of("potion"), 3);  // 아무것도 안 바뀌어요
    const auto notices = f.items.take_notices();
    ASSERT_EQ(notices.size(), 1u);
    EXPECT_EQ(notices[0].kind, LootNotice::Kind::AlreadyFull);
    EXPECT_EQ(notices[0].stack.item_id, "potion");
}

TEST(FieldItems, BusyHeroRefusesAndKeepsThePotion) {
    Fixture f;
    f.hurt();
    f.hero.press_dodge();
    f.hero.step(kDt, {1.0f, 0.0f}, false, f.map);  // 구르는 중
    f.items.request_use(0);
    f.items.step(static_cast<float>(kDt), f.combat, f.hero, f.map);
    EXPECT_EQ(f.bag.count_of("potion"), 3);
    const auto notices = f.items.take_notices();
    ASSERT_EQ(notices.size(), 1u);
    EXPECT_EQ(notices[0].kind, LootNotice::Kind::CannotUse);
}

TEST(FieldItems, HitBeforeGulpWastesThePotion) {
    Fixture f;
    f.hurt();
    f.items.request_use(0);
    f.run(6);                                         // 0.1초 마심
    (void)f.hero.take_hit(10, {1.0f, 0.0f}, 0.0f);    // 꿀꺽 전에 맞음
    f.run(60);
    EXPECT_EQ(f.bag.count_of("potion"), 2);           // 물약은 돌아오지 않아요 (개발자와 정함)
    EXPECT_EQ(f.hero.health().current(), 60);         // 회복도 없어요
}

TEST(FieldItems, NonConsumableOrEmptySlotDoesNothing) {
    Fixture f;
    f.hurt();
    ASSERT_TRUE(f.bag.try_add(f.catalog, "jelly", 1));  // 1번 칸: 젤리
    f.items.request_use(1);
    f.items.request_use(5);  // 빈 칸
    f.run(1);
    EXPECT_EQ(f.bag.count_of("jelly"), 1);
    EXPECT_EQ(f.bag.count_of("potion"), 3);
    EXPECT_TRUE(f.items.take_notices().empty());
    EXPECT_NE(f.hero.state(), wind::game::CharacterState::Drinking);
}

// 단축 칸 (15f): 1을 누르면 등록된 물약이 든 가방 첫 칸에서 하나 꺼내 마셔요. 다 쓰면 아무 일도 없어요
TEST(FieldItems, QuickSlotDrinksTheRegisteredItem) {
    wind::items::ItemCatalog catalog = make_catalog();
    wind::items::Inventory bag;
    wind::game::LootStore store;
    wind::items::QuickSlots quick;
    wind::game::FieldItems items{wind::game::ItemWorld{bag, catalog, store, &quick}, store.on("maps/test.json"), {}, 1};
    const auto map = open_map();
    wind::game::FieldCombat combat{{}, 1, 0.0f};
    PlayerMotion hero{{100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                      wind::game::StaminaTuning{}, 0.15};
    auto run = [&](int steps) {
        for (int i = 0; i < steps; ++i) {
            hero.step(kDt, {}, false, map);
            items.step(static_cast<float>(kDt), combat, hero, map);
        }
    };
    bag.try_add(catalog, "jelly", 1);   // 0번 칸
    bag.try_add(catalog, "potion", 1);  // 1번 칸
    ASSERT_EQ(quick.assign(0, "potion", catalog), wind::items::QuickSlots::Result::Assigned);
    (void)hero.take_hit(30, {1.0f, 0.0f}, 0.0f);
    run(90);  // 경직이 풀릴 때까지

    wind::engine::InputState input;
    const auto bindings = wind::game::InputBindings::defaults();
    input.set_key(wind::engine::Key::Num1, true);  // 1 누름
    items.handle_quick_keys(input, bindings);
    run(1);
    EXPECT_EQ(hero.state(), wind::game::CharacterState::Drinking);
    EXPECT_EQ(bag.count_of("potion"), 0);
    run(60);  // 다 마셔요

    input.begin_frame();
    input.set_key(wind::engine::Key::Num1, false);
    input.begin_frame();
    input.set_key(wind::engine::Key::Num1, true);  // 다시 1: 이제 0개
    items.handle_quick_keys(input, bindings);
    run(1);
    EXPECT_NE(hero.state(), wind::game::CharacterState::Drinking);
    EXPECT_EQ(quick.at(0), "potion");  // 등록은 남아요
}

}  // namespace
