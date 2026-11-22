#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 원본 능력치 다섯 개예요. (명세 8.2, 13a)
//   생명력(vigor) → 최대 HP, 정신력(mind) → 최대 MP, 지구력(endurance) → 최대 스태미나,
//   힘(strength) → 물리 공격력·들 수 있는 무게, 지능(intelligence) → 마법 공격력·마술 칸
// - 각 능력치는 1~60이에요. 시작은 모두 10이에요 (13a, 개발자와 정함).
// - 레벨 = 다섯 능력치의 합 − 49. 모두 10이면 레벨 1, 모두 60이면 레벨 251이에요.
//   레벨은 저장하지 않고 늘 계산해요.
// 그냥 숫자 묶음이라 SDL도 게임 상태도 몰라요.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace wind::progression {

// 능력치 이름표. 순서가 화면(레벨업 표)과 데이터 순서예요.
enum class StatId : std::uint8_t { Vigor, Mind, Endurance, Strength, Intelligence };

inline constexpr std::size_t kStatCount = 5;   // 능력치 수
inline constexpr int kStatMin = 1;             // 능력치 최소
inline constexpr int kStatMax = 60;            // 능력치 최대
inline constexpr int kStartingStat = 10;       // 처음 능력치 (모두 10)
// 레벨 = 합 − kLevelOffset. 모두 시작값이면 레벨 1이 되도록 정해요 (5 × 10 − 1 = 49).
inline constexpr int kLevelOffset = static_cast<int>(kStatCount) * kStartingStat - 1;
inline constexpr int kMaxLevel = static_cast<int>(kStatCount) * kStatMax - kLevelOffset;  // 251

// 다섯 능력치 묶음. stats[StatId::Strength]처럼 이름표로 꺼내요.
struct Stats {
    std::array<int, kStatCount> values{kStartingStat, kStartingStat, kStartingStat, kStartingStat,
                                       kStartingStat};

    [[nodiscard]] int& operator[](StatId id) { return values[static_cast<std::size_t>(id)]; }
    [[nodiscard]] int operator[](StatId id) const { return values[static_cast<std::size_t>(id)]; }
    friend bool operator==(const Stats&, const Stats&) = default;
};

// 모든 능력치 이름표 (반복문용, 순서는 StatId와 같아요)
inline constexpr std::array<StatId, kStatCount> kAllStats{StatId::Vigor, StatId::Mind, StatId::Endurance,
                                                          StatId::Strength, StatId::Intelligence};

// 레벨 = 합 − 49
[[nodiscard]] int level_of(const Stats& stats);
// 모든 능력치가 1~60 안인가요?
[[nodiscard]] bool stats_in_range(const Stats& stats);
// 데이터·로그에 쓰는 영어 이름 ("vigor" 등)
[[nodiscard]] std::string_view stat_key(StatId id);

}  // namespace wind::progression
