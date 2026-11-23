// 이 파일은 무엇을 하나요?
// 필드 룬 담당(쓰러지면 흔적, 걸어가면 되찾기)을 진짜 슬라임 데이터로 숙제 검사(테스트)해요. (명세 8.3.1, 14e)
#include "game/states/field_runes.hpp"

#include "game/entities/player_motion.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/states/field_combat.hpp"

#include "wind/content/monster_loader.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::FieldCombat;
using wind::game::FieldRunes;
using wind::game::HeroProgress;
using wind::game::PlayerMotion;

constexpr double kDt = 1.0 / 60.0;
constexpr const char* kMap = "maps/test.json";

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

PlayerMotion make_hero(float x) {
    return PlayerMotion({x, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15);
}

HeroProgress make_progress(std::int64_t runes) {
    auto progress = HeroProgress::load(WIND_TEST_CONTENT_DIR);
    EXPECT_TRUE(progress.has_value());
    progress->earn(runes);
    return std::move(*progress);
}

// 진짜 슬라임 옆에 가만히 서 있다가 쓰러질 때까지 (덮치기 40 × 3번)
void fall_next_to_slime(PlayerMotion& hero, FieldRunes& runes, HeroProgress& progress) {
    auto data = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    ASSERT_TRUE(data.ok());
    FieldCombat combat({}, 1, 0.0f, {wind::game::Monster("slime", std::move(*data.value), {hero.mover().feet().x + 30.0f, 300.0f})});
    const auto map = open_map();
    for (int i = 0; i < 1800 && !hero.dead(); ++i) {  // 최대 30초
        hero.step(kDt, {}, false, map);
        combat.step(kDt, hero, map);
        runes.step(combat, hero);
    }
    ASSERT_TRUE(hero.dead());
    (void)progress;
}

TEST(FieldRunes, FallingLeavesATraceAtTheFeet) {
    auto progress = make_progress(100);
    FieldRunes runes(progress, kMap, {}, nullptr);
    auto hero = make_hero(100.0f);
    fall_next_to_slime(hero, runes, progress);
    EXPECT_EQ(progress.runes(), 50);
    ASSERT_TRUE(progress.trace().has_value());
    EXPECT_EQ(progress.trace()->map, kMap);
    EXPECT_EQ(progress.trace()->amount, 50);
    EXPECT_FLOAT_EQ(progress.trace()->at.x, hero.mover().feet().x);  // 쓰러진 박자의 발 자리
}

TEST(FieldRunes, WalkingOntoTheTraceRecoversItOnce) {
    auto progress = make_progress(100);
    (void)progress.fall(kMap, {200.0f, 300.0f});  // 흔적 50, 지갑 50
    FieldRunes runes(progress, kMap, {}, nullptr);
    const FieldCombat calm({}, 1, 0.0f);
    const auto far = make_hero(200.0f + FieldRunes::kRecoverRadius + 2.0f);  // 바로 밖
    runes.step(calm, far);
    EXPECT_TRUE(progress.trace().has_value());
    EXPECT_FALSE(runes.take_recovered().has_value());
    const auto near = make_hero(210.0f);
    runes.step(calm, near);
    EXPECT_FALSE(progress.trace().has_value());
    EXPECT_EQ(progress.runes(), 100);  // 전부 되찾았어요
    const auto got = runes.take_recovered();
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(*got, 50);
    runes.step(calm, near);  // 또 서 있어도
    EXPECT_EQ(progress.runes(), 100);
    EXPECT_FALSE(runes.take_recovered().has_value());
}

TEST(FieldRunes, TraceOnAnotherMapIsNotRecoveredHere) {
    auto progress = make_progress(100);
    (void)progress.fall("maps/other.json", {200.0f, 300.0f});
    FieldRunes runes(progress, kMap, {}, nullptr);
    const FieldCombat calm({}, 1, 0.0f);
    runes.step(calm, make_hero(200.0f));  // 같은 좌표라도 다른 맵이에요
    EXPECT_TRUE(progress.trace().has_value());
    EXPECT_EQ(progress.runes(), 50);
}

TEST(FieldRunes, FallenHeroDoesNotRecoverHerOwnTrace) {
    auto progress = make_progress(100);
    FieldRunes runes(progress, kMap, {}, nullptr);
    auto hero = make_hero(100.0f);
    fall_next_to_slime(hero, runes, progress);
    const FieldCombat calm({}, 1, 0.0f);
    for (int i = 0; i < 60; ++i) runes.step(calm, hero);  // 흔적 위에 쓰러져 있어도
    EXPECT_TRUE(progress.trace().has_value());           // 되찾지 않아요 (부활한 뒤 걸어와야 해요)
}

}  // namespace
