#pragma once

// 이 파일은 무엇을 하나요?
// 레벨업 비용(룬)을 구하는 순수 함수예요. (명세 8.3, 13a)
//   레벨 L에서 L+1로 가는 비용 = floor(a + b·(L−1) + c·(L−1)²)
//   기본 a = 100, b = 40, c = 6 → 1→2: 100, 10→11: 946, 30→31: 6,306
// 계수는 데이터(content/progression/levels.json)예요. 레벨은 1~251이라 250→251이 마지막 비용이에요.

#include <cstdint>
#include <optional>

namespace wind::progression {

// 비용 계수 (데이터에서 읽어요). 범위는 불러올 때 검사해요.
struct LevelCostRules {
    double a = 100.0;  // 기본 비용
    double b = 40.0;   // 레벨마다 더해지는 비용
    double c = 6.0;    // 레벨이 오를수록 빨리 커지는 몫
};

// 레벨 level에서 다음 레벨로 가는 비용. level이 1~250이 아니면 nullopt(올릴 수 없음).
[[nodiscard]] std::optional<std::int64_t> level_up_cost(int level, const LevelCostRules& rules);

// 레벨 from에서 to까지 한꺼번에 올리는 비용 (한 칸씩 더해요). from == to이면 0.
// from > to이거나 범위(1~251) 밖이면 nullopt.
[[nodiscard]] std::optional<std::int64_t> cost_between(int from, int to, const LevelCostRules& rules);

}  // namespace wind::progression
