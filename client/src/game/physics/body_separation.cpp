// 이 파일은 무엇을 하나요?
// 겹친 두 몸을 떼어 놓을 거리를 계산하는 곳이에요.

#include "game/physics/body_separation.hpp"

#include <algorithm>

namespace wind::game {

SeparationPush separate_bodies(const BodyCircle& a, const BodyCircle& b, float max_step) {
    const engine::Vec2 between = b.center - a.center;            // a에서 b로
    const float distance = engine::length(between);
    const float overlap = a.radius + b.radius - distance;         // 겹친 깊이
    const float total_weight = std::max(0.0f, a.weight) + std::max(0.0f, b.weight);
    if (!(overlap > 0.0f) || !(max_step > 0.0f) || total_weight <= 0.0f) return {};  // 안 겹쳤거나 밀 수 없음
    // 떼어 놓을 방향: a -> b. 완전히 같은 자리면 오른쪽으로 (늘 같은 결과)
    const engine::Vec2 direction = distance > 0.0f ? between * (1.0f / distance) : engine::Vec2{1.0f, 0.0f};
    const float correction = std::min(overlap, max_step);         // 한 박자에 이만큼까지만 (힘 제한)
    const float a_share = std::max(0.0f, a.weight) / total_weight;  // a가 맡는 몫
    const float b_share = std::max(0.0f, b.weight) / total_weight;  // b가 맡는 몫
    return {direction * (-correction * a_share), direction * (correction * b_share)};
}

}  // namespace wind::game
