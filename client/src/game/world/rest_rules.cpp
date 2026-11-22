// 이 파일은 무엇을 하나요?
// 쉬기 조건 계산: 가장 가까운 쿠션 찾기, 소란스러운지 보기.

#include "game/world/rest_rules.hpp"

namespace wind::game {

std::optional<std::size_t> nearest_cushion(engine::Vec2 feet, const std::vector<Cushion>& cushions,
                                           const RestRules& rules) {
    std::optional<std::size_t> best;
    float best_distance = rules.reach;  // 이보다 멀면 안 돼요 (딱 reach는 돼요)
    for (std::size_t i = 0; i < cushions.size(); ++i) {
        const float distance = engine::length(cushions[i].seat - feet);
        if (distance <= best_distance) {  // 더 가까우면 이 쿠션
            best = i;
            best_distance = distance;
        }
    }
    return best;
}

RestDecision check_rest(engine::Vec2 feet, const std::vector<Cushion>& cushions, bool monster_engaged,
                        float seconds_since_hit, const RestRules& rules) {
    const auto cushion = nearest_cushion(feet, cushions, rules);
    if (!cushion) return {RestCheck::NoCushion, std::nullopt};
    // 몬스터가 노리고 있거나 방금 맞았으면 쉴 수 없어요 (NaN 시간도 "방금"으로 봐요)
    const bool calm = seconds_since_hit >= rules.calm_after_hit;
    if (monster_engaged || !calm) return {RestCheck::TooNoisy, cushion};
    return {RestCheck::Ok, cushion};
}

}  // namespace wind::game
