// 이 파일은 무엇을 하나요?
// 레벨업 비용 계산 (순수 함수).

#include "wind/progression/level_cost.hpp"

#include "wind/progression/stats.hpp"

#include <cmath>

namespace wind::progression {

std::optional<std::int64_t> level_up_cost(int level, const LevelCostRules& rules) {
    if (level < 1 || level >= kMaxLevel) return std::nullopt;  // 251은 더 못 올려요
    const double n = static_cast<double>(level - 1);           // (L − 1)
    const double cost = rules.a + rules.b * n + rules.c * n * n;
    // 계수 범위는 로더가 막지만, 이상한 값(음수·NaN)이 와도 0 아래로 내려가지 않게 해요
    if (!std::isfinite(cost) || cost < 0.0) return std::int64_t{0};
    return static_cast<std::int64_t>(std::floor(cost));
}

std::optional<std::int64_t> cost_between(int from, int to, const LevelCostRules& rules) {
    if (from < 1 || to > kMaxLevel || from > to) return std::nullopt;
    std::int64_t total = 0;
    for (int level = from; level < to; ++level) {  // from→from+1, ..., (to−1)→to
        total += *level_up_cost(level, rules);     // 범위 안이라 늘 값이 있어요
    }
    return total;
}

}  // namespace wind::progression
