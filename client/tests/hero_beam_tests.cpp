// 이 파일은 무엇을 하나요?
// 혜성 광선(15d)을 숙제 검사(테스트)해요: 진짜 마술 파일(comet_beam.json)로
// 0.5초 모으기 → 누르는 동안 유지, 초당 12 MP(그동안 저절로 안 참), 떼면 끝, MP가 떨어지면 끊김,
// 시작하려면 한 번 맞히는 동안의 MP, 모으는 중엔 못 구르고 쏘는 중엔 구르면 끊김,
// 0.1초마다 띠 안의 적을 모두 맞힘(뚫음), 벽에서 잘림.
#include "game/combat/hero_beam.hpp"

#include "game/combat/combat_random.hpp"
#include "game/combat/hero_magic.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/entities/player_motion.hpp"
#include "game/monsters/monster.hpp"

#include "wind/content/spell_loader.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

using wind::game::HeroMagic;
using wind::game::PlayerMotion;
using wind::game::TrainingDummy;

constexpr double kDt = 1.0 / 60.0;
constexpr int kChargeSteps = 30;  // 0.5초
constexpr wind::engine::Vec2 kHeroFeet{100.0f, 300.0f};

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 30;
    map.height = 30;
    map.walkable.assign(900, 1);
    return map;
}

TrainingDummy make_dummy(wind::game::TargetId id, wind::engine::Vec2 feet) {
    wind::combat::TrainingDummyData data;
    data.max_hp = 1000;  // 쓰러지지 않게 넉넉히
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    return TrainingDummy(id, "d" + std::to_string(id), data, feet);
}

// 오른쪽을 보는 주인공(MP 50)이 혜성 광선을 들고 있어요
struct Fixture {
    Fixture() {
        auto spell = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/comet_beam.json");
        EXPECT_TRUE(spell.ok());
        if (spell.ok()) hero.set_spell(*spell.value);
    }
    // 한 박자: 버튼을 누르고 있나(held) → 주인공 → 마술. 이번 박자에 맞힌 수를 돌려줘요
    std::size_t step(bool held = true) {
        hero.hold_cast(held);
        (void)hero.step(kDt, {}, false, map);
        magic.step(static_cast<float>(kDt), hero, map, dummies, monsters, random);
        return magic.hits().size();
    }
    // L을 누르고 다 모을 때까지 (광선이 켜진 박자에서 멈춰요)
    void open_beam() {
        hero.press_cast();
        for (int i = 0; i < kChargeSteps; ++i) step();
        ASSERT_TRUE(hero.caster().beaming());
    }
    void set_mana(float mana) {
        auto vitals = hero.vitals();
        vitals.mana = mana;
        hero.restore(vitals);
    }

    wind::world::MapData map = open_map();
    PlayerMotion hero{kHeroFeet, wind::world::Direction::Right, wind::game::MovementTuning{},
                      wind::game::StaminaTuning{}, 0.15};
    HeroMagic magic{0.0f};  // 치명타 없음
    wind::game::CombatRandom random{1};
    std::vector<TrainingDummy> dummies;
    std::vector<wind::game::Monster> monsters;
};

// 모으는 0.5초 동안은 MP를 안 쓰고, 켜진 뒤 1초에 12씩 써요. 쏘는 동안 MP는 저절로 차지 않아요 (개발자와 정함)
TEST(HeroBeam, DrainsTwelveManaPerSecondWithoutRegen) {
    Fixture f;
    f.open_beam();
    EXPECT_NEAR(f.hero.mana().current(), 50.0f - 12.0f * static_cast<float>(kDt), 0.01f);  // 켜진 박자 몫만
    for (int i = 0; i < 60; ++i) f.step();
    EXPECT_TRUE(f.hero.caster().beaming());
    EXPECT_NEAR(f.hero.mana().current(), 50.0f - 12.0f * 61.0f / 60.0f, 0.05f);  // 1초 더: 약 37.8
}

// 버튼을 떼면 바로 꺼지고, 그 뒤엔 MP가 다시 차요
TEST(HeroBeam, ReleasingStopsTheBeam) {
    Fixture f;
    f.open_beam();
    f.step(false);
    EXPECT_FALSE(f.hero.caster().busy());
    EXPECT_FALSE(f.magic.beam().active());
    const float mana = f.hero.mana().current();
    f.step(false);
    EXPECT_GT(f.hero.mana().current(), mana);  // 다시 차요 (1초에 2)
}

// 완료 기준: MP가 떨어지면 광선이 끊겨요. MP 바도 깜빡여요 (모자람 횟수 +1)
TEST(HeroBeam, BreaksWhenManaRunsOut) {
    Fixture f;
    f.open_beam();
    f.set_mana(6.0f);  // 켜진 뒤 0.5초어치만 남겨요 (모으는 동안은 MP가 차서, 켠 뒤에 맞춰요)
    int beaming_steps = 0;
    while (f.hero.caster().beaming() && beaming_steps < 600) {
        f.step();
        if (f.hero.caster().beaming()) ++beaming_steps;
    }
    EXPECT_NEAR(beaming_steps, 30, 2);  // 약 0.5초 쏘고 끊겨요
    EXPECT_FALSE(f.hero.caster().busy());
    EXPECT_FALSE(f.magic.beam().active());
    EXPECT_EQ(f.hero.mana_denied_count(), 1u);
    EXPECT_LT(f.hero.mana().current(), 0.5f);  // 거의 다 썼어요
}

// 시작하려면 한 번 맞히는 동안의 MP(12 × 0.1 = 1.2)가 있어야 해요. 모자라면 아무 일도 없고 깜빡여요
TEST(HeroBeam, NeedsOneTickOfManaToStart) {
    Fixture f;
    f.set_mana(1.0f);
    f.hero.press_cast();
    f.step();
    EXPECT_FALSE(f.hero.caster().busy());
    EXPECT_EQ(f.hero.mana_denied_count(), 1u);
}

// 모으는 중에는 못 굴러요. 쏘는 중에는 구르면 광선이 끊겨요 (명세 8.5)
TEST(HeroBeam, DodgeOnlyWhileHoldingAndItBreaksTheBeam) {
    Fixture f;
    f.hero.press_cast();
    for (int i = 0; i < 5; ++i) f.step();
    f.hero.press_dodge();  // 모으는 중: 메모는 0.15초 뒤 사라져요
    for (int i = 5; i < kChargeSteps; ++i) {
        f.step();
        EXPECT_NE(f.hero.mover().state(), wind::game::Mover::State::Dodging) << i;
    }
    ASSERT_TRUE(f.hero.caster().beaming());
    f.hero.press_dodge();
    f.step();
    EXPECT_EQ(f.hero.mover().state(), wind::game::Mover::State::Dodging);
    EXPECT_FALSE(f.hero.caster().busy());
}

// 켜진 박자에 띠 안의 적을 모두 맞히고(뚫음), 그 뒤 0.1초마다. 한 번에 12 × 3.0 × 0.1 = 3.6 → 3
TEST(HeroBeam, HitsEveryEnemyInTheLineEveryTick) {
    Fixture f;
    f.dummies.push_back(make_dummy(1, {160.0f, 300.0f}));
    f.dummies.push_back(make_dummy(2, {260.0f, 300.0f}));
    f.dummies.push_back(make_dummy(3, {160.0f, 380.0f}));  // 띠 밖 (아래)
    f.hero.press_cast();
    std::size_t hits = 0;
    for (int i = 0; i < kChargeSteps; ++i) hits = f.step();
    ASSERT_EQ(hits, 2u);  // 켜진 박자: 줄 위의 둘
    for (const auto& hit : f.magic.hits()) EXPECT_EQ(hit.amount, 3);
    for (int i = 0; i < 60; ++i) hits += f.step();  // 1초 더: 0.1초마다 10번 × 2
    EXPECT_EQ(hits, 2u + 20u);
    EXPECT_EQ(f.dummies[2].hp(), f.dummies[2].max_hp());  // 띠 밖은 그대로
}

// 벽에서 잘려요: 벽 뒤 적은 안 맞아요 (개발자와 정함)
TEST(HeroBeam, StopsAtWalls) {
    Fixture f;
    f.map.walkable[9 * 30 + 6] = 0;  // 칸 (6, 9) = x 192~224: 발 높이 줄 위의 벽
    f.dummies.push_back(make_dummy(1, {160.0f, 300.0f}));
    f.dummies.push_back(make_dummy(2, {260.0f, 300.0f}));  // 벽 뒤
    f.open_beam();
    EXPECT_TRUE(f.magic.beam().blocked());
    EXPECT_NEAR(f.magic.beam().length(), 82.0f, 2.0f);  // 손끝 땅 x 110 → 벽 192
    ASSERT_EQ(f.magic.hits().size(), 1u);
    EXPECT_EQ(f.magic.hits()[0].target, 1u);
}

TEST(HeroBeam, RectFollowsTheFacing) {
    const wind::engine::Vec2 hand{100.0f, 50.0f};
    const auto up = wind::game::beam_rect(hand, wind::world::Direction::Up, 80.0f, 14.0f);
    EXPECT_FLOAT_EQ(up.x, 93.0f);
    EXPECT_FLOAT_EQ(up.y, -30.0f);
    EXPECT_FLOAT_EQ(up.w, 14.0f);
    EXPECT_FLOAT_EQ(up.h, 80.0f);
    const auto left = wind::game::beam_rect(hand, wind::world::Direction::Left, 80.0f, 14.0f);
    EXPECT_FLOAT_EQ(left.x, 20.0f);
    EXPECT_FLOAT_EQ(left.y, 43.0f);
}

}  // namespace
