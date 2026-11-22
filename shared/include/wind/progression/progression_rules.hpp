#pragma once

// 이 파일은 무엇을 하나요?
// 성장 규칙 데이터 묶음이에요. (명세 8.2, 8.3, 13a)
// content/progression/stat_curves.json(능력치 곡선)과 levels.json(레벨 비용 계수)을 읽은 결과예요.

#include "wind/progression/derived_stats.hpp"
#include "wind/progression/level_cost.hpp"

namespace wind::progression {

struct ProgressionRules {
    StatCurves curves;      // 능력치 → 최대 HP 등
    LevelCostRules levels;  // 레벨업 비용 계수
};

}  // namespace wind::progression
