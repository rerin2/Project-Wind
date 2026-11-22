// 이 파일은 무엇을 하나요?
// 필드 싸움 담당(FieldCombat)을 저장소의 진짜 롱소드 데이터로 숙제 검사해요. (9단계 완료 조건)
// - 허수아비 앞에서 공격 버튼을 계속 누르면 콤보 3타가 이어지고, HP가 14 -> 15 -> 21만큼 줄어요
// - 칼이 안 닿는 곳에서 휘두르면 HP가 그대로예요
// - 별빛 조각은 첫 허수아비에 12를 주고 사라지고, 벽이나 240픽셀 끝에서도 사라져요 (9e)
// - 맞은 일에 타격감(넉백·멈칫·흔들림)이 실리고, 허수아비가 밀려요. 치명타는 × 1.5 (10a)
// - 진짜 슬라임이 덮치면 주인공이 (공격력 × 위력)만큼 맞고 경직돼요. 한 덮치기에 한 번, 무적이면 안 맞아요. 겹친 몸은 떼어 놓아요 (11c)
// - 칼 콤보로 슬라임을 쓰러뜨리면 사라지고, 별빛 조각도 슬라임에 맞아요. 같은 씨앗·같은 입력 기록이면 같은 싸움 (11d)
// 피해를 정확히 보려는 테스트는 치명타 확률을 0으로 만들어요 (FieldCombat(..., 1, 0.0f)).
#include "game/states/field_combat.hpp"

#include "wind/content/monster_loader.hpp"
#include "wind/content/spell_loader.hpp"
#include "wind/content/weapon_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <optional>
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
        combat.step(kDt, player, map);
        for (const auto& hit : combat.hits()) amounts.push_back(hit.amount);
    }
    return amounts;
}

TEST(FieldCombat, LongswordComboHitsDummyThreeTimes) {
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)}, 1, 0.0f);  // 오른쪽 바로 앞 (칼이 닿는 곳)
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
    FieldCombat combat({make_dummy(260.0f)}, 1, 0.0f);  // 멀리 있어요
    const auto amounts = swing_for(player, combat, 30);
    EXPECT_TRUE(amounts.empty());
    EXPECT_EQ(combat.dummies()[0].hp(), 100);
    EXPECT_FALSE(combat.last_hit().has_value());
    EXPECT_EQ(combat.debug_text(), "dummies 1  shards 0");  // 아직 아무도 안 맞았어요
}

// F3 정보판 싸움 줄: 마지막으로 맞힌 피해, 대상 번호, 남은 HP
TEST(FieldCombat, DebugTextShowsLastHit) {
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)}, 1, 0.0f);
    (void)swing_for(player, combat, 12);           // 1타만 맞을 만큼
    EXPECT_EQ(combat.debug_text(), "dummies 1  shards 0  last hit 14 -> #1 86/100");
}

// ---- 9e: 진짜 별빛 조각 데이터로 쏘기 (9단계 완료 조건) ----

PlayerMotion make_mage() {
    auto player = make_player();
    auto spell = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/star_shard.json");
    EXPECT_TRUE(spell.ok());
    player.set_spell(std::move(*spell.value));
    return player;
}

// 한 번 누르고 steps 박자 동안 흘려보내요. 맞은 양과 사라진 이유를 모아요.
struct ShotResult {
    std::vector<int> amounts;
    std::vector<wind::game::ProjectileEnd> ends;
};

ShotResult cast_and_wait(PlayerMotion& player, FieldCombat& combat,
                         const wind::world::MapData& map, int steps) {
    ShotResult result;
    player.press_cast();
    for (int i = 0; i < steps; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        for (const auto& hit : combat.hits()) result.amounts.push_back(hit.amount);
        for (const auto& end : combat.projectile_ends()) result.ends.push_back(end.reason);
    }
    return result;
}

// 별빛 조각을 쏘면 MP가 6 줄고, 0.25초 뒤 빛 탄이 날아가 허수아비에게 12를 주고 사라져요.
TEST(FieldCombat, StarShardHitsDummyOnceAndVanishes) {
    const auto map = open_map();
    auto player = make_mage();                    // 오른쪽을 보고 서 있어요 (발 100, 300)
    FieldCombat combat({make_dummy(220.0f)}, 1, 0.0f);  // 120픽셀 앞 (칼은 안 닿고 빛 탄은 닿아요)
    const auto result = cast_and_wait(player, combat, map, 60);
    ASSERT_EQ(result.amounts.size(), 1u);         // 한 번만 맞아요
    EXPECT_EQ(result.amounts[0], 12);             // 마법 공격력 12 × 위력 1.0
    EXPECT_EQ(combat.dummies()[0].hp(), 88);
    ASSERT_EQ(result.ends.size(), 1u);
    EXPECT_EQ(result.ends[0], wind::game::ProjectileEnd::Target);
    EXPECT_TRUE(combat.projectiles().empty());
    EXPECT_LT(player.mana().current(), 50.0f - 6.0f + 2.0f + 0.1f);  // 6 쓰고 1초 회복(2)
}

// 앞에 허수아비가 둘 있어도 첫 번째만 맞아요.
TEST(FieldCombat, StarShardHitsOnlyTheFirstTarget) {
    const auto map = open_map();
    auto player = make_mage();
    auto near_dummy = make_dummy(200.0f);
    wind::combat::TrainingDummyData data;
    data.max_hp = 100;
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    TrainingDummy far_dummy(2, "far", data, {240.0f, 300.0f});
    FieldCombat combat({near_dummy, far_dummy}, 1, 0.0f);
    const auto result = cast_and_wait(player, combat, map, 60);
    ASSERT_EQ(result.amounts.size(), 1u);
    EXPECT_EQ(combat.dummies()[0].hp(), 88);
    EXPECT_EQ(combat.dummies()[1].hp(), 100);
}

// 벽에 닿으면 사라지고, 벽 뒤 허수아비는 안 맞아요. (9단계 완료 조건)
TEST(FieldCombat, StarShardStopsAtWall) {
    auto map = open_map();
    map.walkable[static_cast<std::size_t>(9 * map.width + 5)] = 0;  // 발(100, 300) 오른쪽 칸 (5, 9) 벽
    auto player = make_mage();
    FieldCombat combat({make_dummy(220.0f)}, 1, 0.0f);
    const auto result = cast_and_wait(player, combat, map, 60);
    EXPECT_TRUE(result.amounts.empty());
    ASSERT_EQ(result.ends.size(), 1u);
    EXPECT_EQ(result.ends[0], wind::game::ProjectileEnd::Wall);
    EXPECT_EQ(combat.dummies()[0].hp(), 100);
}

// 아무것도 없으면 240픽셀 날고 사라져요. 그 사이 F3 줄에 날고 있는 빛 탄 수가 보여요.
TEST(FieldCombat, StarShardEndsAtRange) {
    const auto map = open_map();
    auto player = make_mage();
    FieldCombat combat({}, 1, 0.0f);
    player.press_cast();
    for (int i = 0; i < 20; ++i) {  // 시전 0.25초 + 조금 날기
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
    }
    ASSERT_EQ(combat.projectiles().size(), 1u);
    EXPECT_EQ(combat.debug_text(), "dummies 0  shards 1");
    const auto rest = cast_and_wait(player, combat, map, 60);  // (MP가 있어 한 발 더 나갈 수 있어서 첫 탄만 봐요)
    ASSERT_FALSE(rest.ends.empty());
    EXPECT_EQ(rest.ends[0], wind::game::ProjectileEnd::Range);
}

// ---- 10a: 타격감 값과 치명타 ----

// 롱소드 1타가 맞으면 공격 데이터의 넉백 90·멈칫 0.05·흔들림 0.15가 맞은 일에 실리고,
// 허수아비 몸통이 바라보는 쪽(오른쪽)으로 밀려요.
TEST(FieldCombat, SwordHitCarriesFeelAndPushesDummy) {
    const auto map = open_map();
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)}, 1, 0.0f);
    std::optional<wind::game::HitEvent> first;
    float max_push = 0.0f;
    for (int i = 0; i < 30; ++i) {
        if (i == 0) player.press_attack();
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        if (!first && !combat.hits().empty()) first = combat.hits().front();
        max_push = std::max(max_push, combat.dummies()[0].body_offset().x);
    }
    ASSERT_TRUE(first.has_value());
    EXPECT_FLOAT_EQ(first->knockback, 90.0f);
    EXPECT_FLOAT_EQ(first->hitstop, 0.05f);
    EXPECT_FLOAT_EQ(first->shake, 0.15f);
    EXPECT_FLOAT_EQ(first->direction.x, 1.0f);
    EXPECT_FALSE(first->critical);
    EXPECT_GT(max_push, 1.0f);   // 오른쪽으로 밀렸다가
    EXPECT_LE(max_push, 8.0f);   // 8픽셀은 넘지 않아요
    EXPECT_FLOAT_EQ(combat.dummies()[0].feet().x, 125.0f);  // 발(칸)은 그대로
}

// 별빛 조각도 마술 데이터의 타격감(넉백 40, 멈칫 0.03)을 실어요.
TEST(FieldCombat, StarShardCarriesSpellFeel) {
    const auto map = open_map();
    auto player = make_mage();
    FieldCombat combat({make_dummy(220.0f)}, 1, 0.0f);
    player.press_cast();
    std::optional<wind::game::HitEvent> hit;
    for (int i = 0; i < 60 && !hit; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        if (!combat.hits().empty()) hit = combat.hits().front();
    }
    ASSERT_TRUE(hit.has_value());
    EXPECT_FLOAT_EQ(hit->knockback, 40.0f);
    EXPECT_FLOAT_EQ(hit->hitstop, 0.03f);
    EXPECT_FLOAT_EQ(hit->direction.x, 1.0f);
}

// 치명타 확률 100%면 피해가 × 1.5 (14 -> 21)이고 맞은 일과 F3 줄에 표시돼요.
TEST(FieldCombat, CriticalHitsDealOneAndAHalf) {
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)}, 1, 1.0f);
    const auto amounts = swing_for(player, combat, 12);  // 1타만
    ASSERT_EQ(amounts.size(), 1u);
    EXPECT_EQ(amounts[0], 21);
    ASSERT_TRUE(combat.last_hit().has_value());
    EXPECT_TRUE(combat.last_hit()->critical);
    EXPECT_EQ(combat.debug_text(), "dummies 1  shards 0  last hit 21! -> #1 79/100");
}

// 같은 씨앗이면 같은 치명타가 나와요 (명세 9.3 재현 가능).
TEST(FieldCombat, SameSeedSameCrits) {
    std::vector<int> first;
    std::vector<int> second;
    for (auto* out : {&first, &second}) {
        auto player = make_player();
        FieldCombat combat({make_dummy(125.0f)}, 2026, 0.5f);
        *out = swing_for(player, combat, 120);
    }
    EXPECT_EQ(first, second);
    EXPECT_FALSE(first.empty());
}


// ---------------------------------------------------------------- 11c: 슬라임이 주인공을 맞혀요 ----

// 저장소의 진짜 슬라임 (공격력 4, 덮치기 40픽셀)
wind::game::Monster make_slime(float feet_x) {
    auto data = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    EXPECT_TRUE(data.ok());
    return wind::game::Monster("slime", std::move(*data.value), {feet_x, 300.0f});
}

// 주인공은 가만히 서 있고, 슬라임만 움직여요. 맞은 일을 모아 돌려줘요.
std::vector<wind::game::HeroHitEvent> stand_for(PlayerMotion& player, FieldCombat& combat, int steps) {
    const auto map = open_map();
    std::vector<wind::game::HeroHitEvent> hits;
    for (int i = 0; i < steps; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        hits.insert(hits.end(), combat.hero_hits().begin(), combat.hero_hits().end());
    }
    return hits;
}

TEST(FieldCombat, SlimeLungeHitsHeroOncePerLunge) {
    auto player = make_player();  // 발 (100, 300)
    FieldCombat combat({}, 1, 0.0f, {make_slime(140.0f)});  // 오른쪽 40픽셀: 알아채자마자 예고
    const auto map = open_map();
    std::vector<wind::game::HeroHitEvent> hits;
    for (int i = 0; i < 120 && hits.empty(); ++i) {  // 첫 번째로 맞을 때까지 (알아채기 0.4 + 예고 0.6 + α)
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        hits = combat.hero_hits();
        if (!hits.empty()) {  // 슬라임 공격 데이터의 멈칫
            EXPECT_FLOAT_EQ(combat.strongest_hitstop(), combat.monsters()[0].attack()->hitstop);
        }
    }
    ASSERT_EQ(hits.size(), 1u);
    // 피해 = 공격력 × 위력 - 방어 0 (최소 1). 숫자는 데이터 파일에서 와서, 슬라임을 고쳐도 규칙만 검사해요
    const auto& slime = combat.monsters()[0];
    const int expected = std::max(1, static_cast<int>(slime.data().attack_power * slime.attack()->power));
    const int expected_hp = std::max(0, 100 - expected);  // HP는 0 아래로 안 내려가요 (12b)
    EXPECT_EQ(hits[0].amount, 100 - expected_hp);
    EXPECT_EQ(player.health().current(), expected_hp);
    EXPECT_LT(hits[0].direction.x, 0.0f);            // 왼쪽으로 덮쳤으니 왼쪽으로 밀려요
    EXPECT_EQ(player.state(), wind::game::CharacterState::Hit);
    // 같은 덮치기에서는 다시 맞지 않아요 (덮치기 0.25초 + 여유)
    EXPECT_TRUE(stand_for(player, combat, 20).empty());
}

TEST(FieldCombat, InvulnerableHeroIsNotHit) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {make_slime(140.0f)});
    const auto map = open_map();
    for (int i = 0; i < 120 && combat.monsters()[0].state() != wind::game::MonsterState::Attack; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
    }
    ASSERT_EQ(combat.monsters()[0].state(), wind::game::MonsterState::Attack);
    (void)player.take_hit(0, {0.0f, 1.0f}, 0.0f);  // 덮치기 직전에 무적 0.6초를 켜 둬요
    EXPECT_TRUE(stand_for(player, combat, 20).empty());  // 덮치기(0.25초) 내내 무적이라 안 맞아요
    EXPECT_EQ(player.health().current(), 100);
}

// 12b: 가만히 서 있으면 슬라임에게 계속 맞아 쓰러지고(마지막 한 방만 fatal),
// 쓰러진 뒤에는 더 맞지 않고 슬라임은 포기하고 제자리로 돌아가요. 쓰러진 몸은 밀어내지 않아요.
TEST(FieldCombat, HeroFallsAndSlimeGoesHome) {
    auto player = make_player();  // 발 (100, 300)
    FieldCombat combat({}, 1, 0.0f, {make_slime(140.0f)});
    const auto map = open_map();
    int fatal = 0;
    int fell_steps = 0;
    int hits_after_death = 0;
    for (int i = 0; i < 60 * 20 && !player.dead(); ++i) {  // 쓰러질 때까지 (최대 20초)
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        for (const auto& hit : combat.hero_hits()) fatal += hit.fatal ? 1 : 0;
        fell_steps += combat.hero_fell() ? 1 : 0;
    }
    ASSERT_TRUE(player.dead());
    EXPECT_EQ(player.health().current(), 0);
    EXPECT_EQ(fatal, 1);       // 마지막 한 방만 fatal
    EXPECT_EQ(fell_steps, 1);  // 쓰러진 박자는 딱 한 번
    for (int i = 0; i < 30; ++i) {  // 마지막 한 방의 넉백(경직 0.3초)이 끝날 때까지
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        hits_after_death += static_cast<int>(combat.hero_hits().size());
    }
    const wind::engine::Vec2 body = player.mover().feet();  // 다 밀려난 뒤 누운 자리
    for (int i = 0; i < 60 * 10; ++i) {  // 10초 더
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        hits_after_death += static_cast<int>(combat.hero_hits().size());
        EXPECT_FALSE(combat.hero_fell());
    }
    EXPECT_EQ(hits_after_death, 0);
    const auto& slime = combat.monsters()[0];
    // 쫓기·공격을 그만두고 제자리 둘레에서 쉬거나 돌아다녀요 (돌아간 뒤에는 다시 알아채지 않아요)
    EXPECT_TRUE(slime.state() == wind::game::MonsterState::Idle || slime.state() == wind::game::MonsterState::Wander)
        << wind::game::to_string(slime.state());
    EXPECT_LE(wind::engine::length(slime.feet() - slime.home()), slime.data().wander.radius + 1.0f);
    EXPECT_NEAR(player.mover().feet().x, body.x, 0.01f);  // 쓰러진 몸은 밀리지 않아요
    EXPECT_NEAR(player.mover().feet().y, body.y, 0.01f);
}

TEST(FieldCombat, BodiesPushApart) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {make_slime(100.0f)});  // 주인공과 같은 자리
    (void)stand_for(player, combat, 10);  // 알아채기 중(움직이지 않음)에 떼어 놓아요
    const float gap = wind::engine::length(combat.monsters()[0].feet() - player.mover().feet());
    EXPECT_GE(gap, 13.0f - 0.01f);  // 반지름 7 + 6
    // 슬라임(가벼움)이 더 많이 밀려요: 주인공 1 : 슬라임 3
    EXPECT_LT(std::abs(player.mover().feet().x - 100.0f), std::abs(combat.monsters()[0].feet().x - 100.0f));
}


// ---------------------------------------------------------------- 11d: 슬라임 때리기·쓰러뜨리기·재현 ----

TEST(FieldCombat, SwordComboKillsSlimeAndItVanishes) {
    auto player = make_player();  // 발 (100, 300), 오른쪽을 봐요
    FieldCombat combat({}, 1, 0.0f, {make_slime(125.0f)});  // 칼이 닿는 곳
    const auto map = open_map();
    std::vector<int> amounts;
    bool vanished = false;
    for (int i = 0; i < 240 && !vanished; ++i) {  // 공격 버튼을 계속 눌러요 (4초)
        player.press_attack();
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        for (const auto& hit : combat.hits()) amounts.push_back(hit.amount);
        vanished = !combat.vanished().empty();
    }
    const auto& slime = combat.monsters()[0];
    EXPECT_TRUE(slime.defeated());
    EXPECT_EQ(slime.hp(), 0);
    ASSERT_GE(amounts.size(), 3u);
    EXPECT_EQ(amounts[0], 14);  // 롱소드 14 -> 15 -> 남은 1 (HP 30)
    EXPECT_EQ(amounts[1], 15);
    EXPECT_EQ(amounts[2], 1);
    EXPECT_TRUE(vanished);      // 쓰러지고 0.5초 뒤 사라짐 (연출이 파티클을 터뜨려요)
    EXPECT_TRUE(slime.gone());
}

TEST(FieldCombat, StarShardHitsSlime) {
    auto spell = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/star_shard.json");
    ASSERT_TRUE(spell.ok());
    auto player = make_player();
    player.set_spell(std::move(*spell.value));
    FieldCombat combat({}, 1, 0.0f, {make_slime(200.0f)});  // 100픽셀 앞 (칼은 안 닿아요)
    const auto map = open_map();
    player.press_cast();
    int total = 0;
    for (int i = 0; i < 90; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        for (const auto& hit : combat.hits()) total += hit.amount;
    }
    EXPECT_EQ(total, 12);                       // 별빛 조각 12
    EXPECT_EQ(combat.monsters()[0].hp(), 18);
}

TEST(FieldCombat, MonsterTargetIdsFollowDummies) {
    TrainingDummy dummy(1, "d", wind::combat::TrainingDummyData{}, {500.0f, 300.0f});
    FieldCombat combat({dummy}, 1, 0.0f, {make_slime(125.0f), make_slime(400.0f)});
    EXPECT_EQ(combat.monsters()[0].id(), 2u);  // 허수아비 1 다음부터
    EXPECT_EQ(combat.monsters()[1].id(), 3u);
}

// 같은 씨앗 + 같은 박자별 주인공 입력 기록이면 같은 결과 (명세 17.2). 치명타(5%)도 켜 둬요.
TEST(FieldCombat, SameSeedSameInputsSameFight) {
    struct Result {
        wind::engine::Vec2 hero;
        int hero_hp = 0;
        std::vector<wind::engine::Vec2> slimes;
        std::vector<int> slime_hp;
        std::vector<wind::game::MonsterState> states;
        int hits = 0;
        int hero_hits = 0;
    };
    const auto replay = [](std::uint64_t seed) {
        const auto map = open_map();
        auto player = make_player();
        FieldCombat combat({}, seed, wind::game::kBaseCritChance, {make_slime(220.0f), make_slime(260.0f)});
        Result result;
        for (int i = 0; i < 60 * 12; ++i) {  // 12초 입력 기록
            // 입력 기록: 앞으로 걷다 서기를 되풀이하고, 0.5초마다 공격, 3초마다 구르기
            const int beat = i % 120;
            const wind::engine::Vec2 move = beat < 50 ? wind::engine::Vec2{1.0f, 0.0f}
                                                      : (beat < 70 ? wind::engine::Vec2{-1.0f, 0.0f} : wind::engine::Vec2{});
            if (i % 30 == 0) player.press_attack();
            if (i % 180 == 90) player.press_dodge();
            player.step(kDt, move, false, map);
            combat.step(kDt, player, map);
            result.hits += static_cast<int>(combat.hits().size());
            result.hero_hits += static_cast<int>(combat.hero_hits().size());
            for (const auto& m : combat.monsters()) result.states.push_back(m.state());
        }
        result.hero = player.mover().feet();
        result.hero_hp = player.health().current();
        for (const auto& m : combat.monsters()) {
            result.slimes.push_back(m.feet());
            result.slime_hp.push_back(m.hp());
        }
        return result;
    };
    const auto a = replay(2026);
    const auto b = replay(2026);
    EXPECT_EQ(a.hero, b.hero);
    EXPECT_EQ(a.hero_hp, b.hero_hp);
    EXPECT_EQ(a.slimes, b.slimes);
    EXPECT_EQ(a.slime_hp, b.slime_hp);
    EXPECT_EQ(a.states, b.states);
    // 이 기록에서는 양쪽이 실제로 맞아요 (재현 테스트가 싸움을 다루는지 확인)
    EXPECT_GT(a.hits, 0);
    EXPECT_GT(a.hero_hits, 0);
}

}  // namespace

// ---------------------------------------------------------------- 13a: 룬과 능력치 보너스 ----

TEST(FieldCombat, KillingSlimeEarnsItsRunesOnce) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {make_slime(125.0f)});  // 칼이 닿는 곳
    const auto map = open_map();
    const auto reward = combat.monsters()[0].data().rune_reward;  // 슬라임 데이터 값 (12)
    ASSERT_GT(reward, 0);
    std::int64_t total = 0;
    int paying_steps = 0;
    for (int i = 0; i < 240; ++i) {  // 쓰러뜨리고 사라질 때까지 (4초)
        player.press_attack();
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        if (combat.runes_earned() > 0) ++paying_steps;
        total += combat.runes_earned();
    }
    EXPECT_TRUE(combat.monsters()[0].defeated());
    EXPECT_EQ(total, reward);    // 정확히 한 마리 몫
    EXPECT_EQ(paying_steps, 1);  // 쓰러진 박자 한 번만
}

TEST(FieldCombat, StrengthBonusRaisesSwordDamage) {
    auto player = make_player();
    FieldCombat combat({make_dummy(125.0f)}, 1, 0.0f);
    wind::progression::DerivedStats derived;
    derived.strength_bonus = 0.5f;  // 힘 20
    combat.set_hero_stats(derived);
    const auto amounts = swing_for(player, combat, 30);
    ASSERT_FALSE(amounts.empty());
    EXPECT_EQ(amounts[0], 19);  // 14 × (1 + 0.8 × 0.5) = 19.6 → 19 (1타 위력 1.0)
}

// ---------------------------------------------------------------- 13b: 쉬기 조건 ----

TEST(FieldCombat, MonsterEngagedWhileSlimeNoticesHero) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {make_slime(160.0f)});  // 60픽셀 옆: 감지 거리 안
    const auto map = open_map();
    EXPECT_FALSE(combat.monster_engaged());  // 아직 쉬는 중
    bool engaged = false;
    for (int i = 0; i < 30 && !engaged; ++i) {
        player.step(kDt, {}, false, map);
        combat.step(kDt, player, map);
        engaged = combat.monster_engaged();
    }
    EXPECT_TRUE(engaged);
}
