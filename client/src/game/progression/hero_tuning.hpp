#pragma once

// 이 파일은 무엇을 하나요?
// 파생값(최대 HP·MP·스태미나)을 주인공 몸의 숫자 묶음(Tuning)으로 옮기는 도우미 함수 모음이에요. (13a)
// 최대값만 파생값에서 오고, 회복 빠르기 같은 나머지 숫자는 각 Tuning의 기본값 그대로예요.

#include "game/entities/health.hpp"
#include "game/entities/mana.hpp"
#include "game/entities/stamina.hpp"

#include "wind/progression/derived_stats.hpp"

namespace wind::game {

// 최대 HP = 생명력 곡선
[[nodiscard]] HealthTuning health_tuning(const progression::DerivedStats& derived);
// 최대 MP = 정신력 곡선
[[nodiscard]] ManaTuning mana_tuning(const progression::DerivedStats& derived);
// 최대 스태미나 = 지구력 곡선
[[nodiscard]] StaminaTuning stamina_tuning(const progression::DerivedStats& derived);

}  // namespace wind::game
