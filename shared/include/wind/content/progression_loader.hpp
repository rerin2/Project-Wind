#pragma once

// 이 파일은 무엇을 하나요?
// 성장 규칙 파일 두 개를 읽고 숙제 검사를 해요. (명세 8.2, 8.3, 13a)
//
// content/progression/stat_curves.json — 곡선마다 [능력치, 값] 점 목록
//   { "schema_version": 1,
//     "max_hp": [[10, 100], [20, 190], [30, 280], [40, 360], [60, 450]],
//     "max_mp": [...], "max_stamina": [...], "carry_weight": [...],
//     "strength_bonus": [...], "intelligence_bonus": [...], "spell_slots": [...] }
//   검사: 곡선마다 점 1~20개, 능력치는 1~60 정수이고 점마다 커져야(오름차순, 같은 값 안 됨) 해요,
//         값은 0~99999.
//
// content/progression/levels.json — 레벨 비용 계수 (비용 = floor(a + b·(L−1) + c·(L−1)²))
//   { "schema_version": 1, "a": 100, "b": 40, "c": 6 }
//   검사: a 1~1,000,000 (비용이 늘 1 이상), b 0~100,000, c 0~10,000.

#include "wind/content/load_result.hpp"
#include "wind/progression/derived_stats.hpp"
#include "wind/progression/level_cost.hpp"
#include "wind/progression/progression_rules.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>

namespace wind::content {

inline constexpr std::int64_t kStatCurvesSchemaVersion = 1;
inline constexpr std::int64_t kLevelRulesSchemaVersion = 1;
inline constexpr const char* kStatCurvesPath = "progression/stat_curves.json";
inline constexpr const char* kLevelRulesPath = "progression/levels.json";

[[nodiscard]] LoadResult<progression::StatCurves> parse_stat_curves(const nlohmann::json& document,
                                                                    const std::string& file);
[[nodiscard]] LoadResult<progression::LevelCostRules> parse_level_rules(const nlohmann::json& document,
                                                                        const std::string& file);

// 두 파일을 다 읽어요. 하나라도 틀리면 실패하고, 두 파일의 오류를 모두 돌려줘요.
[[nodiscard]] LoadResult<progression::ProgressionRules> load_progression_rules(
    const std::filesystem::path& content_root);

}  // namespace wind::content
