// 이 파일은 무엇을 하나요?
// 주인공 마술이 나간 뒤(15c)를 숙제 검사(테스트)해요: 진짜 마술 파일로
// 달빛 칼날(앞 상자 한 번 베기), 별무리(돌기 → 가까운 적 쪽으로 / 적이 없으면 시전 방향, 도는 별에 닿으면 맞고 사라짐),
// 큰 별빛 조각(시전 중 멈춤), 휘기 한도.
#include "game/combat/hero_magic.hpp"

#include "game/combat/combat_random.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/entities/player_motion.hpp"
#include "game/monsters/monster.hpp"

#include "wind/content/spell_loader.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

namespace {

using wind::game::HeroMagic;
using wind::game::PlayerMotion;
using wind::game::TrainingDummy;

constexpr double kDt = 1.0 / 60.0;
constexpr wind::engine::Vec2 kHeroFeet{100.0f, 300.0f};

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 30;
    map.height = 30;
    map.walkable.assign(900, 1);
    return map;
}

wind::combat::Spell real_spell(const std::string& id) {
    auto spell = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/" + id + ".json");
    EXPECT_TRUE(spell.ok()) << id;
    return spell.ok() ? *spell.value : wind::combat::Spell{};
}

TrainingDummy make_dummy(wind::game::TargetId id, wind::engine::Vec2 feet) {
    wind::combat::TrainingDummyData data;
    data.max_hp = 1000;  // 쓰러지지 않게 넉넉히
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    return TrainingDummy(id, "d" + std::to_string(id), data, feet);
}

// 오른쪽을 보는 주인공과 마술 담당, 허수아비들
struct Fixture {
    explicit Fixture(const std::string& spell_id) { hero.set_spell(real_spell(spell_id)); }
    // 한 박자: 주인공(move 방향으로) → 마술. 이번 박자에 맞힌 수를 돌려줘요
    std::size_t step(wind::engine::Vec2 move = {}) {
        (void)hero.step(kDt, move, false, map);
        magic.step(static_cast<float>(kDt), hero, map, dummies, monsters, random);
        return magic.hits().size();
    }

    wind::world::MapData map = open_map();
    PlayerMotion hero{kHeroFeet, wind::world::Direction::Right, wind::game::MovementTuning{},
                      wind::game::StaminaTuning{}, 0.15};
    HeroMagic magic{0.0f};  // 치명타 없음
    wind::game::CombatRandom random{1};
    std::vector<TrainingDummy> dummies;
    std::vector<wind::game::Monster> monsters;
};

}  // namespace

TEST(HeroMagic, MoonBladeCutsEveryTargetInFrontOnce) {
    Fixture f("moon_blade");
    f.dummies.push_back(make_dummy(1, {118.0f, 300.0f}));  // 바로 앞
    f.dummies.push_back(make_dummy(2, {128.0f, 300.0f}));  // 조금 더 앞 (상자 안)
    f.dummies.push_back(make_dummy(3, {60.0f, 300.0f}));   // 뒤
    f.hero.press_cast();
    std::size_t hits = 0;
    int slashes = 0;
    for (int i = 0; i < 40; ++i) {
        hits += f.step();
        slashes += static_cast<int>(f.magic.slashes().size());
    }
    EXPECT_EQ(hits, 2u);    // 앞의 둘만, 한 번씩
    EXPECT_EQ(slashes, 1);  // 칼자국 연출도 한 번
    EXPECT_EQ(f.dummies[2].hp(), 1000);
    EXPECT_LT(f.dummies[0].hp(), 1000);
}

TEST(HeroMagic, StarSwarmOrbitsThenFliesTowardTheNearestEnemy) {
    Fixture f("star_swarm");
    f.dummies.push_back(make_dummy(1, {100.0f, 180.0f}));  // 위쪽 120px (시전은 오른쪽)
    f.hero.press_cast();
    int ticks = 0;
    while (f.magic.projectiles().empty() && ticks < 60) { f.step(); ++ticks; }
    ASSERT_EQ(f.magic.projectiles().size(), 5u);  // 작은 별 다섯
    EXPECT_TRUE(f.magic.projectiles().front().orbiting());
    for (int i = 0; i < 80; ++i) f.step();         // 1.5초가 다 되기 전에는 모두 돌아요
    for (const auto& star : f.magic.projectiles()) EXPECT_TRUE(star.orbiting());
    // 첫 별이 떠날 때까지
    const wind::game::Projectile* flying = nullptr;
    for (int i = 0; i < 40 && flying == nullptr; ++i) {
        f.step();
        for (const auto& star : f.magic.projectiles()) {
            if (!star.orbiting() && star.travelled() > 0.0f) flying = &star;
        }
    }
    ASSERT_NE(flying, nullptr);
    EXPECT_LT(flying->shot().direction.y, -0.5f);  // 오른쪽이 아니라 위(적 쪽)로
    std::size_t hits = 0;
    for (int i = 0; i < 120; ++i) hits += f.step();
    EXPECT_GE(hits, 3u);  // 휘어서 대부분 맞아요
}

TEST(HeroMagic, StarSwarmWithoutEnemiesFliesTheCastDirection) {
    Fixture f("star_swarm");
    f.hero.press_cast();
    const wind::game::Projectile* flying = nullptr;
    for (int i = 0; i < 160 && flying == nullptr; ++i) {
        f.step();
        for (const auto& star : f.magic.projectiles()) {
            if (!star.orbiting() && star.travelled() > 0.0f) flying = &star;
        }
    }
    ASSERT_NE(flying, nullptr);
    EXPECT_NEAR(flying->shot().direction.x, 1.0f, 1e-4f);
    EXPECT_NEAR(flying->shot().direction.y, 0.0f, 1e-4f);
}

// 개발자와 정함: 도는 별에 닿으면 맞고 그 별은 사라져요
TEST(HeroMagic, OrbitingStarHitsWhatItTouchesAndVanishes) {
    Fixture f("star_swarm");
    f.dummies.push_back(make_dummy(1, {120.0f, 300.0f}));  // 도는 길에 걸쳐 있어요
    f.hero.press_cast();
    std::size_t hits = 0;
    int ended = 0;
    for (int i = 0; i < 100; ++i) {  // 아직 아무 별도 떠나지 않았을 때까지 (0.4 + 1.5초 전)
        hits += f.step();
        ended += static_cast<int>(f.magic.ended().size());
    }
    EXPECT_GE(hits, 1u);
    EXPECT_EQ(static_cast<std::size_t>(ended), hits);           // 맞힌 별마다 하나씩 사라짐
    EXPECT_EQ(f.magic.projectiles().size() + hits, 5u);
}

TEST(HeroMagic, GreatStarShardRootsTheCaster) {
    Fixture great("great_star_shard");
    Fixture small("star_shard");
    const wind::engine::Vec2 right{1.0f, 0.0f};
    great.hero.press_cast();
    small.hero.press_cast();
    for (int i = 0; i < 12; ++i) {  // 둘 다 시전 중 (0.25초 = 15박자 전)
        great.step(right);
        small.step(right);
    }
    ASSERT_TRUE(great.hero.caster().busy());
    ASSERT_TRUE(small.hero.caster().busy());
    EXPECT_LT(great.hero.mover().feet().x - kHeroFeet.x, 0.5f);  // 큰 별빛 조각: 멈춤 (cast_move_scale 0)
    EXPECT_GT(small.hero.mover().feet().x - kHeroFeet.x, 3.0f);  // 별빛 조각: 천천히 걸어요 (0.4배)
}

TEST(HeroMagic, SteeringTurnsNoMoreThanHomingAllows) {
    wind::game::ProjectileShot shot;
    shot.direction = {1.0f, 0.0f};
    shot.data.speed = 100.0f;
    shot.data.range = 500.0f;
    shot.data.homing = 0.5f;  // 1초에 최대 0.5 × 8 = 4 라디안
    wind::game::Projectile star(shot);
    star.steer({0.0f, -100.0f}, 0.1f);  // 바로 뒤 위쪽으로 가고 싶어도 0.4 라디안만
    const float angle = std::atan2(star.shot().direction.y, star.shot().direction.x);
    EXPECT_NEAR(std::fabs(angle), 0.4f, 1e-3f);
    shot.data.homing = 0.0f;
    wind::game::Projectile straight(shot);
    straight.steer({0.0f, -100.0f}, 0.1f);
    EXPECT_FLOAT_EQ(straight.shot().direction.x, 1.0f);  // homing 0이면 그대로
}
