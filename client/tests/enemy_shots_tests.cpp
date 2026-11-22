// 이 파일은 무엇을 하나요?
// 새 몬스터 세 종류 검사 (13c, 진짜 데이터 파일)
// - 개구리 침이 날아와 주인공을 맞히고, 맞으면 사라져요 / 주인공이 무적이면 그냥 지나가요
// - 가시 버섯의 가시는 몸 둘레 어느 쪽이든 닿아요
// - 박쥐는 슬라임처럼 예고 뒤 덮쳐요
#include "game/combat/enemy_shots.hpp"

#include "game/entities/player_motion.hpp"
#include "game/monsters/monster.hpp"
#include "game/states/field_combat.hpp"

#include "wind/content/monster_loader.hpp"

#include <gtest/gtest.h>

using wind::engine::Vec2;
using wind::game::FieldCombat;
using wind::game::PlayerMotion;

namespace {

constexpr double kDt = 1.0 / 60.0;
const Vec2 kHero{320.0f, 320.0f};

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 40;
    map.height = 30;
    map.walkable.assign(static_cast<std::size_t>(40 * 30), 1);
    return map;
}

wind::game::Monster load(const char* path, Vec2 home) {
    auto data = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, path);
    EXPECT_TRUE(data.ok()) << path;
    return wind::game::Monster("m", data.ok() ? *data.value : wind::combat::MonsterData{}, home);
}

PlayerMotion make_player() {
    return PlayerMotion(kHero, wind::world::Direction::Down, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15);
}

// 주인공은 가만히, 싸움만 돌려요. 주인공이 처음 맞은 박자 수 (못 맞으면 -1)
int first_hero_hit(PlayerMotion& player, FieldCombat& combat, int steps, bool keep_invulnerable = false) {
    const auto map = open_map();
    for (int i = 0; i < steps; ++i) {
        if (keep_invulnerable && i % 8 == 0) player.press_dodge();  // 계속 굴러서 무적 (구르기 처음 0.16초)
        player.step(kDt, keep_invulnerable ? Vec2{0.0f, 1.0f} : Vec2{}, false, map);
        combat.step(kDt, player, map);
        if (!combat.hero_hits().empty()) return i;
    }
    return -1;
}

}  // namespace

TEST(EnemyShots, FrogSpitFliesAndHitsHero) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {load("monsters/frog.json", kHero + Vec2{100.0f, 0.0f})});  // 100픽셀 옆
    const auto map = open_map();
    bool saw_shot = false;
    int hit_step = -1;
    for (int i = 0; i < 300 && hit_step < 0; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        saw_shot = saw_shot || !combat.enemy_shots().shots().empty();
        if (!combat.hero_hits().empty()) hit_step = i;
    }
    EXPECT_TRUE(saw_shot);                       // 침이 날아왔어요
    ASSERT_GE(hit_step, 0);                      // 그리고 맞혔어요
    EXPECT_LT(player.health().current(), player.health().max());
    EXPECT_TRUE(combat.enemy_shots().shots().empty());  // 맞힌 침은 사라져요
}

TEST(EnemyShots, InvulnerableHeroLetsSpitPass) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {load("monsters/frog.json", kHero + Vec2{100.0f, 0.0f})});
    EXPECT_EQ(first_hero_hit(player, combat, 200, true), -1);  // 계속 구르면(무적) 안 맞아요
}

TEST(EnemyShots, MushroomSpikesHitFromAnySide) {
    for (const Vec2 offset : {Vec2{28.0f, 0.0f}, Vec2{-28.0f, 0.0f}, Vec2{0.0f, 26.0f}, Vec2{0.0f, -26.0f}}) {
        auto player = make_player();
        FieldCombat combat({}, 1, 0.0f, {load("monsters/mushroom.json", kHero + offset)});
        EXPECT_GE(first_hero_hit(player, combat, 300), 0) << offset.x << "," << offset.y;
    }
}

TEST(EnemyShots, BatDivesLikeSlime) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {load("monsters/bat.json", kHero + Vec2{50.0f, 0.0f})});
    EXPECT_GE(first_hero_hit(player, combat, 300), 0);
}
