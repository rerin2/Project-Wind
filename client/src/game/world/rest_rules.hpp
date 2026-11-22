#pragma once

// 이 파일은 무엇을 하나요?
// "지금 쿠션에서 쉴 수 있나?"를 정하는 순수 함수예요. (명세 8.7, 13b)
// - 쿠션 자리에서 24픽셀 안에 서 있어야 해요 (가장 가까운 쿠션을 골라요).
// - 주인공을 노리는 몬스터가 있거나(알아채기~움찔), 맞은 지 3초가 안 지났으면 "너무 소란스러움"이에요.
// 같은 값을 넣으면 늘 같은 답이라 창 없이 테스트할 수 있어요.

#include "game/world/cushion.hpp"

#include "engine/core/math.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace wind::game {

// 쉬기 조건 숫자 (명세 8.7)
struct RestRules {
    float reach = 24.0f;           // 쿠션 자리에서 이 거리 안이면 쉴 수 있어요 (픽셀)
    float calm_after_hit = 3.0f;   // 맞은 뒤 이만큼 지나야 쉴 수 있어요 (초)
};

enum class RestCheck : std::uint8_t {
    NoCushion,  // 가까운 쿠션이 없어요 (E를 눌러도 아무 일 없음)
    TooNoisy,   // 쿠션은 있지만 주변이 너무 소란스러워요 (알림)
    Ok,         // 쉴 수 있어요
};

struct RestDecision {
    RestCheck result = RestCheck::NoCushion;
    std::optional<std::size_t> cushion;  // 고른 쿠션 번호 (NoCushion이면 없음)
};

// 발에서 reach 안의 가장 가까운 쿠션 번호 (없으면 nullopt). "[E] 쉬기" 안내에도 써요.
[[nodiscard]] std::optional<std::size_t> nearest_cushion(engine::Vec2 feet, const std::vector<Cushion>& cushions,
                                                         const RestRules& rules = {});

// 쉴 수 있나요? monster_engaged: 주인공을 노리는 몬스터가 있나, seconds_since_hit: 맞은 뒤 지난 시간
[[nodiscard]] RestDecision check_rest(engine::Vec2 feet, const std::vector<Cushion>& cushions, bool monster_engaged,
                                      float seconds_since_hit, const RestRules& rules = {});

}  // namespace wind::game
