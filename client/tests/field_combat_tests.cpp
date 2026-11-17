// 이 파일은 무엇을 하나요?
// 필드 싸움 담당(FieldCombat)을 저장소의 진짜 롱소드 데이터로 숙제 검사해요. (9단계 완료 조건)
// - 허수아비 앞에서 공격 버튼을 계속 누르면 콤보 3타가 이어지고, HP가 14 -> 15 -> 21만큼 줄어요
// - 칼이 안 닿는 곳에서 휘두르면 HP가 그대로예요
// - 별빛 조각은 첫 허수아비에 12를 주고 사라지고, 벽이나 240픽셀 끝에서도 사라져요 (9e)
// - 맞은 일에 타격감(넉백·멈칫·흔들림)이 실리고, 허수아비가 밀려요. 치명타는 × 1.5 (10a)
// - 진짜 슬라임이 덮치면 주인공이 4 맞고 경직돼요. 한 덮치기에 한 번, 무적이면 안 맞아요. 겹친 몸은 떼어 놓아요 (11c)
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
        if (!hits.empty()) EXPECT_FLOAT_EQ(combat.strongest_hitstop(), 0.06f);  // 슬라임 공격의 멈칫
    }
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_EQ(hits[0].amount, 4);                    // 공격력 4 × 위력 1 - 방어 0
    EXPECT_EQ(player.health().current(), 96);
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

TEST(FieldCombat, BodiesPushApart) {
    auto player = make_player();
    FieldCombat combat({}, 1, 0.0f, {make_slime(100.0f)});  // 주인공과 같은 자리
    (void)stand_for(player, combat, 10);  // 알아채기 중(움직이지 않음)에 떼어 놓아요
    const float gap = wind::engine::length(combat.monsters()[0].feet() - player.mover().feet());
    EXPECT_GE(gap, 13.0f - 0.01f);  // 반지름 7 + 6
    // 슬라임(가벼움)이 더 많이 밀려요: 주인공 1 : 슬라임 3
    EXPECT_LT(std::abs(player.mover().feet().x - 100.0f), std::abs(combat.monsters()[0].feet().x - 100.0f));
}

}  // namespace
