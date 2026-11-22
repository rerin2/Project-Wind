// 이 파일은 무엇을 하나요?
// 능력치 묶음의 작은 계산(레벨, 범위 검사, 이름)이에요.

#include "wind/progression/stats.hpp"

#include <algorithm>
#include <numeric>

namespace wind::progression {

int level_of(const Stats& stats) {
    // 다섯 값을 더하고 49를 빼요 (모두 10이면 1)
    return std::accumulate(stats.values.begin(), stats.values.end(), 0) - kLevelOffset;
}

bool stats_in_range(const Stats& stats) {
    return std::ranges::all_of(stats.values, [](int v) { return v >= kStatMin && v <= kStatMax; });
}

std::string_view stat_key(StatId id) {
    switch (id) {
    case StatId::Vigor: return "vigor";
    case StatId::Mind: return "mind";
    case StatId::Endurance: return "endurance";
    case StatId::Strength: return "strength";
    case StatId::Intelligence: return "intelligence";
    }
    return "unknown";  // 여기 올 일은 없어요 (이름표가 다섯 개뿐)
}

}  // namespace wind::progression
