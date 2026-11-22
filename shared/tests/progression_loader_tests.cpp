// 이 파일은 무엇을 하나요?
// 성장 규칙 파일 읽기 검사 (13a)
// - 저장소의 진짜 파일 두 개가 통과하나
// - 곡선: 능력치 범위 밖, 거꾸로 된 순서, 음수 값, 모양이 틀린 점, 빠진 곡선은 거절하나
// - 레벨 비용: 범위 밖 계수는 거절하나
#include "wind/content/progression_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;

json make_curves() {  // 통과하는 곡선 파일 (테스트마다 한 칸씩 망가뜨려요)
    const json line = json::array({json::array({10, 1}), json::array({60, 2})});
    return json{{"schema_version", 1}, {"max_hp", line},         {"max_mp", line},
                {"max_stamina", line},  {"carry_weight", line},   {"strength_bonus", line},
                {"intelligence_bonus", line}, {"spell_slots", line}};
}

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

}  // namespace

TEST(ProgressionLoader, RealFilesLoad) {
    const auto rules = wind::content::load_progression_rules(WIND_TEST_CONTENT_DIR);
    ASSERT_TRUE(rules.ok());
    EXPECT_DOUBLE_EQ(rules.value->levels.a, 100.0);
    EXPECT_DOUBLE_EQ(rules.value->levels.b, 40.0);
    EXPECT_DOUBLE_EQ(rules.value->levels.c, 6.0);
    EXPECT_EQ(rules.value->curves.max_hp.points().size(), 5u);
}

TEST(ProgressionLoader, GoodCurvesPass) {
    EXPECT_TRUE(wind::content::parse_stat_curves(make_curves(), "c.json").ok());
}

TEST(ProgressionLoader, RejectsStatOutOfRange) {
    auto doc = make_curves();
    doc["max_hp"][1][0] = 61;
    const auto result = wind::content::parse_stat_curves(doc, "c.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "max_hp[1]"));
    doc = make_curves();
    doc["max_mp"][0][0] = 0;
    EXPECT_FALSE(wind::content::parse_stat_curves(doc, "c.json").ok());
}

TEST(ProgressionLoader, RejectsPointsOutOfOrder) {
    auto doc = make_curves();
    doc["max_stamina"] = json::array({json::array({20, 1}), json::array({20, 2})});  // 같은 능력치 두 번
    auto result = wind::content::parse_stat_curves(doc, "c.json");
    EXPECT_TRUE(has_error_at(result.errors, "max_stamina[1]"));
    doc["max_stamina"] = json::array({json::array({30, 1}), json::array({20, 2})});  // 거꾸로
    EXPECT_FALSE(wind::content::parse_stat_curves(doc, "c.json").ok());
}

TEST(ProgressionLoader, RejectsNegativeAndBadPoints) {
    auto doc = make_curves();
    doc["carry_weight"][0][1] = -1;
    EXPECT_FALSE(wind::content::parse_stat_curves(doc, "c.json").ok());
    doc = make_curves();
    doc["spell_slots"][0] = json::array({10});  // 값이 없어요
    EXPECT_TRUE(has_error_at(wind::content::parse_stat_curves(doc, "c.json").errors, "spell_slots[0]"));
    doc = make_curves();
    doc["spell_slots"][0] = json::array({10.5, 2});  // 능력치는 정수
    EXPECT_FALSE(wind::content::parse_stat_curves(doc, "c.json").ok());
    doc = make_curves();
    doc["max_hp"] = json::array();  // 점이 하나도 없어요
    EXPECT_FALSE(wind::content::parse_stat_curves(doc, "c.json").ok());
}

TEST(ProgressionLoader, RejectsMissingCurve) {
    auto doc = make_curves();
    doc.erase("intelligence_bonus");
    EXPECT_FALSE(wind::content::parse_stat_curves(doc, "c.json").ok());
}

TEST(ProgressionLoader, LevelRulesRanges) {
    const json good{{"schema_version", 1}, {"a", 100}, {"b", 40}, {"c", 6}};
    EXPECT_TRUE(wind::content::parse_level_rules(good, "l.json").ok());
    auto bad = good;
    bad["a"] = 0;  // 비용이 0이면 공짜 레벨업이 돼요
    EXPECT_FALSE(wind::content::parse_level_rules(bad, "l.json").ok());
    bad = good;
    bad["c"] = -1;
    EXPECT_FALSE(wind::content::parse_level_rules(bad, "l.json").ok());
    bad = good;
    bad["b"] = 100001;
    EXPECT_FALSE(wind::content::parse_level_rules(bad, "l.json").ok());
    bad = good;
    bad.erase("b");
    EXPECT_FALSE(wind::content::parse_level_rules(bad, "l.json").ok());
}
