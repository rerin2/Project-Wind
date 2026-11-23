// 이 파일은 무엇을 하나요?
// 필드의 무기 담당(15a)을 숙제 검사(테스트)해요:
// 시작 무기 쥐여 주기, 가방에서 바꿔 끼우기(전부 성공 또는 전부 실패), 무거움(힘 10의 대검), 레벨업 뒤 무거움 풀림,
// 무기마다 휘두르는 속도·스태미나가 뚜렷이 다른지, 단검은 휘두르며 걷는지.
#include "game/states/field_arms.hpp"

#include "game/entities/player_motion.hpp"
#include "game/combat/spell_shelf.hpp"
#include "game/items/loot_store.hpp"
#include "game/items/weapon_rack.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/items/equipment.hpp"
#include "wind/items/inventory.hpp"
#include "wind/progression/stats.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::LootNotice;
using wind::game::PlayerMotion;
using wind::items::EquipSlot;

constexpr double kDt = 1.0 / 60.0;

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

// 진짜 도감·무기 파일·성장 규칙으로 만든 무기 담당과 주인공
struct Fixture {
    wind::items::ItemCatalog catalog = *wind::content::load_item_catalog(WIND_TEST_CONTENT_DIR).value;
    wind::items::Inventory bag;
    wind::game::LootStore store;
    wind::items::Equipment worn;
    wind::game::WeaponRack weapons = wind::game::WeaponRack::load(WIND_TEST_CONTENT_DIR, catalog);
    wind::game::SpellShelf spells;  // 무기 담당은 마술을 안 써요
    wind::game::HeroProgress progress = *wind::game::HeroProgress::load(WIND_TEST_CONTENT_DIR);
    wind::game::FieldArms arms{wind::game::ItemWorld{bag, catalog, store}, wind::game::GearWorld{worn, weapons, spells},
                               progress};
    wind::world::MapData map = open_map();
    PlayerMotion hero{{100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                      wind::game::StaminaTuning{}, 0.15};

    Fixture() {
        worn.wear(EquipSlot::Weapon, "longsword");  // 시작 무기 (명세 8.1)
        arms.arm(hero);
    }
    void step() { (void)hero.step(kDt, {}, false, map); }
    // 장착 요청 → 다음 박자에 처리 → 알림 하나
    LootNotice equip(std::size_t slot) {
        arms.request_equip(slot);
        arms.step(hero);
        const auto notices = arms.take_notices();
        EXPECT_EQ(notices.size(), 1u);
        return notices.empty() ? LootNotice{} : notices.front();
    }
    // 한 번 휘두르는 데 걸린 박자 수와 쓴 스태미나 (스태미나는 휘두르기 시작한 박자에 재요: 긴 공격 중에는 다시 차기 시작해요)
    std::pair<int, float> swing() {
        const float before = hero.stamina().current();
        hero.press_attack();
        step();
        const float spent = before - hero.stamina().current();
        int ticks = 1;
        while (hero.attack().busy() && ticks < 600) {
            step();
            ++ticks;
        }
        return {ticks, spent};
    }
};

}  // namespace

TEST(FieldArms, StartsWithTheWornLongsword) {
    Fixture f;
    EXPECT_EQ(f.hero.attack().weapon().id, "longsword");
    EXPECT_FALSE(f.arms.held().heavy);
}

TEST(FieldArms, EquipSwapsWithTheBagSlot) {
    Fixture f;
    ASSERT_TRUE(f.bag.try_add(f.catalog, "dagger", 1));
    const auto notice = f.equip(0);
    EXPECT_EQ(notice.kind, LootNotice::Kind::Equipped);
    EXPECT_EQ(notice.stack.item_id, "dagger");
    EXPECT_EQ(f.hero.attack().weapon().id, "dagger");
    EXPECT_EQ(f.worn.worn(EquipSlot::Weapon), "dagger");
    ASSERT_TRUE(f.bag.at(0).has_value());
    EXPECT_EQ(f.bag.at(0)->item_id, "longsword");  // 벗은 롱소드는 그 칸으로
}

// 완료 기준: 힘이 모자란 대검은 무거움 (알림도 붉은 "너무 무겁다")
TEST(FieldArms, GreatswordIsHeavyAtStartingStrength) {
    Fixture f;
    ASSERT_TRUE(f.bag.try_add(f.catalog, "greatsword", 1));
    const auto notice = f.equip(0);
    EXPECT_EQ(notice.kind, LootNotice::Kind::EquippedHeavy);
    EXPECT_TRUE(notice.warning());
    EXPECT_TRUE(f.arms.held().heavy);
    const auto& held = f.hero.attack().weapon();
    EXPECT_FLOAT_EQ(held.stamina_per_attack, 26.0f * 1.5f);
    EXPECT_FLOAT_EQ(held.combo[0].windup, 0.22f * 1.3f);
}

TEST(FieldArms, RaisingStrengthLiftsTheGreatsword) {
    Fixture f;
    ASSERT_TRUE(f.bag.try_add(f.catalog, "greatsword", 1));
    (void)f.equip(0);
    ASSERT_TRUE(f.arms.held().heavy);
    auto stronger = f.progress.stats();
    stronger[wind::progression::StatId::Strength] = 20;  // 들 수 있는 무게 14
    f.progress.earn(999'999);
    ASSERT_TRUE(f.progress.try_raise(stronger));
    f.arms.arm(f.hero);  // 레벨업 뒤 필드가 다시 쥐여 줘요 (refresh_hero_stats)
    EXPECT_FALSE(f.arms.held().heavy);
    EXPECT_FLOAT_EQ(f.hero.attack().weapon().stamina_per_attack, 26.0f);
}

TEST(FieldArms, CannotSwapMidSwingAndNothingChanges) {
    Fixture f;
    ASSERT_TRUE(f.bag.try_add(f.catalog, "dagger", 1));
    f.hero.press_attack();
    f.step();
    ASSERT_TRUE(f.hero.attack().busy());
    const auto notice = f.equip(0);
    EXPECT_EQ(notice.kind, LootNotice::Kind::CannotEquip);
    EXPECT_EQ(f.hero.attack().weapon().id, "longsword");
    EXPECT_EQ(f.worn.worn(EquipSlot::Weapon), "longsword");
    EXPECT_EQ(f.bag.at(0)->item_id, "dagger");
}

TEST(FieldArms, MaterialIsNotEquippedAndBagIsUntouched) {
    Fixture f;
    ASSERT_TRUE(f.bag.try_add(f.catalog, "slime_jelly", 3));
    f.arms.request_equip(0);
    f.arms.step(f.hero);
    EXPECT_TRUE(f.arms.take_notices().empty());
    EXPECT_EQ(f.bag.at(0)->count, 3);
    EXPECT_EQ(f.worn.worn(EquipSlot::Weapon), "longsword");
}

// 완료 기준: 단검·롱소드·대검은 휘두르는 시간과 스태미나가 눈에 띄게 달라요
TEST(FieldArms, WeaponsFeelClearlyDifferent) {
    Fixture f;
    const auto [sword_ticks, sword_stamina] = f.swing();
    ASSERT_TRUE(f.bag.try_add(f.catalog, "dagger", 1));
    ASSERT_TRUE(f.bag.try_add(f.catalog, "greatsword", 1));
    (void)f.equip(0);  // 단검 (롱소드는 0번 칸으로)
    f.hero.restore({f.hero.health().current(), f.hero.mana().current(), f.hero.stamina().max()});
    const auto [dagger_ticks, dagger_stamina] = f.swing();
    (void)f.equip(1);  // 대검 (무거움)
    f.hero.restore({f.hero.health().current(), f.hero.mana().current(), f.hero.stamina().max()});
    const auto [great_ticks, great_stamina] = f.swing();
    EXPECT_LT(dagger_ticks * 3, sword_ticks * 2);  // 단검은 롱소드보다 1/3 넘게 빨라요
    EXPECT_GT(great_ticks, sword_ticks * 2);       // 무거운 대검은 두 배 넘게 느려요
    EXPECT_FLOAT_EQ(dagger_stamina, 8.0f);
    EXPECT_FLOAT_EQ(sword_stamina, 14.0f);
    EXPECT_FLOAT_EQ(great_stamina, 39.0f);
}

TEST(FieldArms, DaggerKeepsWalkingWhileSwingingButLongswordStops) {
    Fixture f;
    const wind::engine::Vec2 right{1.0f, 0.0f};
    f.hero.press_attack();
    for (int i = 0; i < 10; ++i) (void)f.hero.step(kDt, right, false, f.map);
    const float sword_speed = wind::engine::length(f.hero.mover().velocity());
    for (int i = 0; i < 60; ++i) f.step();  // 끝까지 쉬어요
    ASSERT_TRUE(f.bag.try_add(f.catalog, "dagger", 1));
    (void)f.equip(0);
    f.hero.press_attack();
    for (int i = 0; i < 10; ++i) (void)f.hero.step(kDt, right, false, f.map);
    ASSERT_TRUE(f.hero.attack().busy());
    const float dagger_speed = wind::engine::length(f.hero.mover().velocity());
    EXPECT_LT(sword_speed, 1.0f);
    EXPECT_GT(dagger_speed, 20.0f);
    EXPECT_EQ(f.hero.mover().facing(), wind::world::Direction::Right);
}
