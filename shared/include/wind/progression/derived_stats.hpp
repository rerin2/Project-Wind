#pragma once

// 이 파일은 무엇을 하나요?
// 능력치에서 나오는 값(파생값)을 구하는 "한 곳의 계산 함수"예요. (명세 8.2, 22.2 단계 13–15)
// 최대 HP·MP·스태미나, 들 수 있는 무게, 힘·지능 보너스, 마술 칸 수를
// 능력치 곡선 데이터(content/progression/stat_curves.json)에서 구해요.
// 다른 곳에서 이 값을 따로 계산하면 안 돼요. 장비·효과(15단계)도 나중에 이 함수에 더해요.
// 순수 함수라 같은 능력치면 언제나 같은 답이에요.

#include "wind/progression/stat_curve.hpp"
#include "wind/progression/stats.hpp"

namespace wind::progression {

// 파생값마다 곡선 하나. 어떤 능력치가 어느 곡선을 쓰는지는 derive_stats가 정해요 (명세 8.2 표).
struct StatCurves {
    StatCurve max_hp;              // 생명력 → 최대 HP
    StatCurve max_mp;              // 정신력 → 최대 MP
    StatCurve max_stamina;         // 지구력 → 최대 스태미나
    StatCurve carry_weight;        // 힘 → 들 수 있는 무게
    StatCurve strength_bonus;      // 힘 → 힘 보너스 (물리 공격력)
    StatCurve intelligence_bonus;  // 지능 → 지능 보너스 (마법 공격력)
    StatCurve spell_slots;         // 지능 → 마술 칸 수
};

// 능력치에서 나온 값 묶음
struct DerivedStats {
    int max_hp = 0;                  // 최대 HP (소수점 버림)
    int max_mp = 0;                  // 최대 MP (소수점 버림)
    int max_stamina = 0;             // 최대 스태미나 (소수점 버림)
    float carry_weight = 0.0f;       // 들 수 있는 무게
    float strength_bonus = 0.0f;     // 힘 보너스 (물리 공격력 = 기본 × (1 + 힘 보정 × 이 값))
    float intelligence_bonus = 0.0f; // 지능 보너스 (마법 공격력 = 12 × (1 + 이 값))
    int spell_slots = 0;             // 마술 칸 수 (소수점 버림)
    friend bool operator==(const DerivedStats&, const DerivedStats&) = default;
};

// 능력치 → 파생값. 능력치가 1~60 밖이면 곡선 끝 값을 써요 (곡선이 평평하게 늘여요).
[[nodiscard]] DerivedStats derive_stats(const Stats& stats, const StatCurves& curves);

}  // namespace wind::progression
