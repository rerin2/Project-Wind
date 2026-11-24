// 이 파일은 무엇을 하나요?
// 필드의 마술 담당(15b)과 마술 선반을 숙제 검사(테스트)해요:
// 고른 마술 쥐여 주기, Q로 다음 칸(시전 중이면 무시), 지능이 오르면 칸이 열림, 혜성 광선도 모으기 시작함(15d).
#include "game/states/field_spells.hpp"

#include "game/combat/spell_shelf.hpp"
#include "game/entities/player_motion.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/progression/stats.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::PlayerMotion;
using wind::progression::StatId;

constexpr double kDt = 1.0 / 60.0;

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

// 진짜 마술 파일·성장 규칙. intelligence: 시작 지능
struct Fixture {
    explicit Fixture(int intelligence = 20) {
        wind::progression::Stats stats;
        stats[StatId::Intelligence] = intelligence;
        progress.emplace(*wind::game::HeroProgress::load(WIND_TEST_CONTENT_DIR, stats));
        for (const char* id : {"star_shard", "great_star_shard", "moon_blade", "comet_beam"}) {
            EXPECT_TRUE(shelf.load(WIND_TEST_CONTENT_DIR, id)) << id;
        }
        spells.emplace(*progress, shelf);
    }
    // 배우고 빈 칸에 넣어요 (시작 설정과 같은 규칙)
    wind::progression::SpellBook::SlotResult learn(const char* id) {
        auto& book = progress->spells();
        book.learn(id);
        return book.put_in_free_slot(id, shelf.find(id)->intelligence_required,
                                     progress->stats()[StatId::Intelligence],
                                     static_cast<std::size_t>(progress->derived().spell_slots));
    }
    void step() { (void)hero.step(kDt, {}, false, map); }

    wind::game::SpellShelf shelf;
    std::optional<wind::game::HeroProgress> progress;
    std::optional<wind::game::FieldSpells> spells;
    wind::world::MapData map = open_map();
    PlayerMotion hero{{100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                      wind::game::StaminaTuning{}, 0.15};
};

}  // namespace

TEST(SpellShelf, LoadsByIdAndRejectsMissingFiles) {
    wind::game::SpellShelf shelf;
    EXPECT_TRUE(shelf.load(WIND_TEST_CONTENT_DIR, "moon_blade"));
    EXPECT_TRUE(shelf.load(WIND_TEST_CONTENT_DIR, "moon_blade"));  // 두 번째는 그대로
    EXPECT_EQ(shelf.size(), 1u);
    EXPECT_FALSE(shelf.load(WIND_TEST_CONTENT_DIR, "no_such_spell"));
    EXPECT_EQ(shelf.find("no_such_spell"), nullptr);
}

TEST(FieldSpells, ArmsTheSelectedSpellAndQCycles) {
    Fixture f;  // 지능 20: 칸 3개
    ASSERT_EQ(f.learn("star_shard"), wind::progression::SpellBook::SlotResult::Done);
    ASSERT_EQ(f.learn("great_star_shard"), wind::progression::SpellBook::SlotResult::Done);
    f.spells->arm(f.hero);
    ASSERT_NE(f.hero.caster().spell(), nullptr);
    EXPECT_EQ(f.hero.caster().spell()->id, "star_shard");

    f.spells->press_next();
    EXPECT_TRUE(f.spells->step(f.hero));
    EXPECT_EQ(f.hero.caster().spell()->id, "great_star_shard");
    EXPECT_EQ(f.spells->switch_count(), 1u);
    f.spells->press_next();
    EXPECT_TRUE(f.spells->step(f.hero));  // 끝에서 처음으로
    EXPECT_EQ(f.hero.caster().spell()->id, "star_shard");
    EXPECT_FALSE(f.spells->step(f.hero));  // 누르지 않으면 그대로
}

TEST(FieldSpells, QWhileCastingIsIgnored) {
    Fixture f;
    f.learn("star_shard");
    f.learn("great_star_shard");
    f.spells->arm(f.hero);
    f.hero.press_cast();
    f.step();
    ASSERT_TRUE(f.hero.caster().busy());
    f.spells->press_next();
    EXPECT_FALSE(f.spells->step(f.hero));
    EXPECT_EQ(f.hero.caster().spell()->id, "star_shard");
    for (int i = 0; i < 30; ++i) f.step();  // 시전이 끝나도 버린 Q가 다시 오지 않아요
    EXPECT_FALSE(f.spells->step(f.hero));
    EXPECT_EQ(f.hero.caster().spell()->id, "star_shard");
}

// 완료 기준: 지능이 모자란 마술은 마술 칸에 넣을 수 없음 (시작 지능 10)
TEST(FieldSpells, StartingIntelligenceCannotSlotNewSpells) {
    Fixture f(10);
    EXPECT_EQ(f.learn("star_shard"), wind::progression::SpellBook::SlotResult::Done);
    EXPECT_EQ(f.learn("moon_blade"), wind::progression::SpellBook::SlotResult::NeedsIntelligence);
    EXPECT_EQ(f.learn("great_star_shard"), wind::progression::SpellBook::SlotResult::NeedsIntelligence);
    EXPECT_TRUE(f.progress->spells().knows("moon_blade"));  // 배우기는 했어요
    f.spells->press_next();
    EXPECT_FALSE(f.spells->step(f.hero));  // 칸에는 별빛 조각뿐
}

// 혜성 광선(15d)도 칸에서 고르면 L로 모으기 시작해요. 모으는 동안은 MP를 안 써요
TEST(FieldSpells, CometBeamStartsChargingWithoutSpendingMana) {
    Fixture f(40);  // 지능 40: 칸 4개, 혜성 광선(36)도 돼요
    f.learn("comet_beam");  // 빈 공책이라 첫 칸에 들어가요
    f.spells->arm(f.hero);
    ASSERT_EQ(f.hero.caster().spell()->id, "comet_beam");
    const float mana = f.hero.mana().current();
    f.hero.press_cast();
    f.hero.hold_cast(true);
    f.step();
    EXPECT_TRUE(f.hero.caster().busy());
    EXPECT_FALSE(f.hero.caster().beaming());
    EXPECT_GE(f.hero.mana().current(), mana);  // 안 줄었어요 (조금 차기만 해요)
}
