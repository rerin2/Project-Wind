// 이 파일은 무엇을 하나요?
// 능력치 → 파생값 계산 (한 곳). 어떤 능력치가 어느 곡선을 쓰는지는 명세 8.2 표와 같아요.

#include "wind/progression/derived_stats.hpp"

#include <cmath>

namespace wind::progression {
namespace {

// 정수로 쓰는 값(최대 HP 등)의 소수점 버림.
// 곡선 계산에서 109.99999처럼 아주 조금 모자라게 나오는 일이 있어서, 아주 작은 값을 더한 뒤 버려요.
constexpr double kFloorTolerance = 1e-6;

int floor_int(double value) {
    return static_cast<int>(std::floor(value + kFloorTolerance));
}

}  // namespace

DerivedStats derive_stats(const Stats& stats, const StatCurves& curves) {
    DerivedStats d;
    d.max_hp = floor_int(curves.max_hp.value_at(stats[StatId::Vigor]));               // 생명력
    d.max_mp = floor_int(curves.max_mp.value_at(stats[StatId::Mind]));                // 정신력
    d.max_stamina = floor_int(curves.max_stamina.value_at(stats[StatId::Endurance])); // 지구력
    d.carry_weight = static_cast<float>(curves.carry_weight.value_at(stats[StatId::Strength]));     // 힘
    d.strength_bonus = static_cast<float>(curves.strength_bonus.value_at(stats[StatId::Strength])); // 힘
    d.intelligence_bonus =
        static_cast<float>(curves.intelligence_bonus.value_at(stats[StatId::Intelligence]));        // 지능
    d.spell_slots = floor_int(curves.spell_slots.value_at(stats[StatId::Intelligence]));          // 지능
    return d;
}

}  // namespace wind::progression
