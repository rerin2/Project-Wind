// 이 파일은 무엇을 하나요?
// 마술 파일 검사(명세 8.6, 14.2)를 숙제 검사(테스트)해요.
// - 맞는 마술은 칸이 그대로 읽혀요 (별빛 조각, 광선, 근접)
// - MP 칸은 둘 중 하나만, 동작은 셋 중 하나만
// - 범위 밖 숫자, 콘텐츠 폴더 밖 경로, 모르는 버전은 거절
// - 진짜 content/spells/star_shard.json이 읽혀요
#include "wind/content/spell_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <string_view>

namespace {

using nlohmann::json;
using wind::combat::SpellMotion;
using wind::content::parse_spell;

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 별빛 조각과 같은 맞는 마술 하나
json make_doc() {
    return json{{"schema_version", 1},
                {"id", "star_shard"},
                {"name", "별빛 조각"},
                {"intelligence_required", 10},
                {"mp_cost", 6},
                {"cast_seconds", 0.25},
                {"power", 1.0},
                {"projectile", {{"speed", 360}, {"range", 240}, {"size", {6, 6}}}}};
}

// 혜성 광선처럼 1초마다 MP를 쓰는 광선 마술
json make_beam() {
    auto doc = make_doc();
    doc.erase("mp_cost");
    doc.erase("projectile");
    doc["mp_per_second"] = 12;
    doc["beam"] = {{"length", 160}, {"width", 12}, {"tick_seconds", 0.1}};
    return doc;
}

TEST(SpellLoader, ParsesProjectileSpell) {
    auto doc = make_doc();
    doc["light"] = {{"radius", 36}, {"color", "#a8dcef"}, {"intensity", 0.8}};
    doc["fx"] = "fx/star_trail.json";
    const auto result = parse_spell(doc, "spells/s.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& s = *result.value;
    EXPECT_EQ(s.id, "star_shard");
    EXPECT_EQ(s.name, "별빛 조각");
    EXPECT_EQ(s.intelligence_required, 10);
    EXPECT_EQ(s.mp_cost, 6);
    EXPECT_FLOAT_EQ(s.mp_per_second, 0.0f);
    EXPECT_FLOAT_EQ(s.cast_seconds, 0.25f);
    EXPECT_FLOAT_EQ(s.power, 1.0f);
    EXPECT_EQ(s.motion, SpellMotion::Projectile);
    ASSERT_TRUE(s.projectile.has_value());
    EXPECT_FLOAT_EQ(s.projectile->speed, 360.0f);
    EXPECT_FLOAT_EQ(s.projectile->range, 240.0f);
    EXPECT_FLOAT_EQ(s.projectile->width, 6.0f);
    EXPECT_FLOAT_EQ(s.projectile->height, 6.0f);
    EXPECT_EQ(s.projectile->count, 1);             // 기본값
    EXPECT_FLOAT_EQ(s.projectile->homing, 0.0f);  // 기본값
    EXPECT_FALSE(s.melee.has_value());
    EXPECT_FALSE(s.beam.has_value());
    ASSERT_TRUE(s.light.has_value());
    EXPECT_FLOAT_EQ(s.light->radius, 36.0f);
    EXPECT_EQ(s.light->color.r, 0xa8);
    EXPECT_EQ(s.fx, "fx/star_trail.json");
}

TEST(SpellLoader, ParsesBeamAndMeleeSpells) {
    const auto beam = parse_spell(make_beam(), "b.json");
    ASSERT_TRUE(beam.ok()) << wind::content::to_string(beam.errors.front());
    EXPECT_EQ(beam.value->motion, SpellMotion::Beam);
    EXPECT_FLOAT_EQ(beam.value->mp_per_second, 12.0f);
    EXPECT_EQ(beam.value->mp_cost, 0);
    ASSERT_TRUE(beam.value->beam.has_value());
    EXPECT_FLOAT_EQ(beam.value->beam->length, 160.0f);

    auto melee_doc = make_doc();
    melee_doc.erase("projectile");
    melee_doc["melee"] = {{"offset", {20, -8}}, {"size", {32, 40}}};
    const auto melee = parse_spell(melee_doc, "m.json");
    ASSERT_TRUE(melee.ok()) << wind::content::to_string(melee.errors.front());
    EXPECT_EQ(melee.value->motion, SpellMotion::Melee);
    ASSERT_TRUE(melee.value->melee.has_value());
    EXPECT_FLOAT_EQ(melee.value->melee->length, 32.0f);
}

// 타격감 선택 칸 (10a): 없으면 0, 있으면 범위 검사
TEST(SpellLoader, ReadsOptionalHitFeel) {
    const auto plain = parse_spell(make_doc(), "s.json");
    ASSERT_TRUE(plain.ok());
    EXPECT_FLOAT_EQ(plain.value->knockback, 0.0f);
    EXPECT_FLOAT_EQ(plain.value->hitstop, 0.0f);
    EXPECT_FLOAT_EQ(plain.value->shake, 0.0f);

    auto doc = make_doc();
    doc["knockback"] = 40;
    doc["hitstop"] = 0.03;
    doc["shake"] = 0.05;
    const auto felt = parse_spell(doc, "s.json");
    ASSERT_TRUE(felt.ok());
    EXPECT_FLOAT_EQ(felt.value->knockback, 40.0f);
    EXPECT_FLOAT_EQ(felt.value->hitstop, 0.03f);
    EXPECT_FLOAT_EQ(felt.value->shake, 0.05f);

    auto long_stop = make_doc();
    long_stop["hitstop"] = 0.6;  // 0.5초가 최대
    EXPECT_TRUE(has_error_at(parse_spell(long_stop, "s.json").errors, "hitstop"));
    auto backwards = make_doc();
    backwards["knockback"] = -1;
    EXPECT_TRUE(has_error_at(parse_spell(backwards, "s.json").errors, "knockback"));
    auto wild = make_doc();
    wild["shake"] = 2;
    EXPECT_TRUE(has_error_at(parse_spell(wild, "s.json").errors, "shake"));
}

// 별무리처럼 여러 개·유도·돌기 칸도 읽혀요 (움직임은 15단계)
TEST(SpellLoader, ReadsOptionalProjectileFields) {
    auto doc = make_doc();
    doc["projectile"]["count"] = 5;
    doc["projectile"]["homing"] = 0.8;
    doc["projectile"]["orbit_seconds"] = 1.5;
    const auto result = parse_spell(doc, "s.json");
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value->projectile->count, 5);
    EXPECT_FLOAT_EQ(result.value->projectile->homing, 0.8f);
    EXPECT_FLOAT_EQ(result.value->projectile->orbit_seconds, 1.5f);
}

// MP 칸은 꼭 하나만
TEST(SpellLoader, NeedsExactlyOneMpField) {
    auto both = make_doc();
    both["mp_per_second"] = 3;
    EXPECT_TRUE(has_error_at(parse_spell(both, "s.json").errors, "mp_cost"));

    auto none = make_doc();
    none.erase("mp_cost");
    EXPECT_TRUE(has_error_at(parse_spell(none, "s.json").errors, "mp_cost"));
}

// 동작은 꼭 하나만
TEST(SpellLoader, NeedsExactlyOneMotion) {
    auto two = make_doc();
    two["beam"] = {{"length", 160}, {"width", 12}, {"tick_seconds", 0.1}};
    EXPECT_TRUE(has_error_at(parse_spell(two, "s.json").errors, "projectile"));

    auto none = make_doc();
    none.erase("projectile");
    EXPECT_TRUE(has_error_at(parse_spell(none, "s.json").errors, "projectile"));
}

// 1초마다 쓰는 MP는 광선만, 광선은 1초마다만
TEST(SpellLoader, MpStyleMustMatchMotion) {
    auto beam_with_cost = make_beam();
    beam_with_cost.erase("mp_per_second");
    beam_with_cost["mp_cost"] = 10;
    EXPECT_TRUE(has_error_at(parse_spell(beam_with_cost, "s.json").errors, "mp_cost"));

    auto shard_per_second = make_doc();
    shard_per_second.erase("mp_cost");
    shard_per_second["mp_per_second"] = 5;
    EXPECT_TRUE(has_error_at(parse_spell(shard_per_second, "s.json").errors, "mp_per_second"));
}

// 경계값: 요구 지능 1~60, MP 1~999, 빛 탄 빠르기 1 이상
TEST(SpellLoader, RejectsOutOfRangeNumbers) {
    for (const int intelligence : {0, 61}) {
        auto doc = make_doc();
        doc["intelligence_required"] = intelligence;
        EXPECT_TRUE(has_error_at(parse_spell(doc, "s.json").errors, "intelligence_required"))
            << intelligence;
    }
    for (const int ok : {1, 60}) {
        auto doc = make_doc();
        doc["intelligence_required"] = ok;
        EXPECT_TRUE(parse_spell(doc, "s.json").ok()) << ok;
    }
    auto free = make_doc();
    free["mp_cost"] = 0;
    EXPECT_TRUE(has_error_at(parse_spell(free, "s.json").errors, "mp_cost"));

    auto frozen = make_doc();
    frozen["projectile"]["speed"] = 0;
    EXPECT_TRUE(has_error_at(parse_spell(frozen, "s.json").errors, "projectile.speed"));

    auto huge = make_doc();
    huge["projectile"]["size"] = {6, 500};
    EXPECT_TRUE(has_error_at(parse_spell(huge, "s.json").errors, "projectile.size"));

    auto swarm = make_doc();
    swarm["projectile"]["count"] = 0;
    EXPECT_TRUE(has_error_at(parse_spell(swarm, "s.json").errors, "projectile.count"));

    auto bright = make_doc();
    bright["light"] = {{"radius", 36}, {"color", "#a8dcef"}, {"intensity", 1.5}};
    EXPECT_TRUE(has_error_at(parse_spell(bright, "s.json").errors, "light.intensity"));
}

TEST(SpellLoader, RejectsUnsafeOrUnknown) {
    auto escape = make_doc();
    escape["fx"] = "../../secret.json";
    EXPECT_FALSE(parse_spell(escape, "s.json").ok());

    auto version = make_doc();
    version["schema_version"] = 2;
    EXPECT_FALSE(parse_spell(version, "s.json").ok());

    EXPECT_FALSE(parse_spell(json::array(), "s.json").ok());
    EXPECT_FALSE(parse_spell(json{{"schema_version", 1}}, "s.json").ok());
}

TEST(SpellLoader, LoadFileStaysInsideContentRoot) {
    const auto result = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "../CMakeLists.txt");
    EXPECT_FALSE(result.ok());
}

// 진짜 게임 데이터: 별빛 조각 (명세 8.6 표, 9d에서 정한 속도 360·거리 240·크기 6x6)
TEST(SpellLoader, RealStarShardLoads) {
    const auto result =
        wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/star_shard.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& s = *result.value;
    EXPECT_EQ(s.name, "별빛 조각");
    EXPECT_EQ(s.mp_cost, 6);
    EXPECT_FLOAT_EQ(s.cast_seconds, 0.25f);
    EXPECT_FLOAT_EQ(s.power, 1.0f);
    ASSERT_TRUE(s.projectile.has_value());
    EXPECT_FLOAT_EQ(s.projectile->speed, 360.0f);
    EXPECT_FLOAT_EQ(s.projectile->range, 240.0f);
    EXPECT_FLOAT_EQ(s.projectile->width, 6.0f);
    ASSERT_TRUE(s.light.has_value());
    EXPECT_FLOAT_EQ(s.knockback, 40.0f);  // 10a: 칼보다 약한 타격감
    EXPECT_FLOAT_EQ(s.hitstop, 0.03f);
}

}  // namespace

// HUD 아이콘 칸 (15b): 없으면 0, 0~255만 돼요
TEST(SpellLoader, ReadsOptionalIcon) {
    auto doc = make_doc();
    EXPECT_EQ(wind::content::parse_spell(doc, "s.json").value->icon, 0);
    doc["icon"] = 3;
    EXPECT_EQ(wind::content::parse_spell(doc, "s.json").value->icon, 3);
    doc["icon"] = 300;
    EXPECT_FALSE(wind::content::parse_spell(doc, "s.json").ok());
}

// 저장소의 마술 다섯 개 (명세 8.6 표): 요구 지능·MP·시전·위력·동작이 표와 같아요
TEST(SpellLoader, ShippedSpellsMatchTheTable) {
    struct Row {
        const char* id;
        int intelligence;
        float cast;
        float power;
        wind::combat::SpellMotion motion;
    };
    using wind::combat::SpellMotion;
    const Row rows[] = {{"star_shard", 10, 0.25f, 1.0f, SpellMotion::Projectile},
                        {"great_star_shard", 16, 0.6f, 2.2f, SpellMotion::Projectile},
                        {"star_swarm", 20, 0.4f, 0.6f, SpellMotion::Projectile},
                        {"moon_blade", 14, 0.15f, 1.4f, SpellMotion::Melee},
                        {"comet_beam", 36, 0.5f, 3.0f, SpellMotion::Beam}};
    for (const auto& row : rows) {
        const auto spell = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR,
                                                          std::string("spells/") + row.id + ".json");
        ASSERT_TRUE(spell.ok()) << row.id;
        EXPECT_EQ(spell.value->id, row.id);
        EXPECT_EQ(spell.value->intelligence_required, row.intelligence) << row.id;
        EXPECT_FLOAT_EQ(spell.value->cast_seconds, row.cast) << row.id;
        EXPECT_FLOAT_EQ(spell.value->power, row.power) << row.id;
        EXPECT_EQ(spell.value->motion, row.motion) << row.id;
    }
    const auto swarm = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/star_swarm.json");
    EXPECT_EQ(swarm.value->projectile->count, 5);
    EXPECT_FLOAT_EQ(swarm.value->projectile->orbit_seconds, 1.5f);
    const auto beam = wind::content::load_spell_file(WIND_TEST_CONTENT_DIR, "spells/comet_beam.json");
    EXPECT_FLOAT_EQ(beam.value->mp_per_second, 12.0f);
}
