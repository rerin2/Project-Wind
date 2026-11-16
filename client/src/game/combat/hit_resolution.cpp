// 이 파일은 무엇을 하나요?
// 칼 한 번이 허수아비들에 맞았는지 보고 피해를 주는 곳이에요.

#include "game/combat/hit_resolution.hpp"

namespace wind::game {

std::vector<HitEvent> resolve_swing(const Swing& swing, std::span<TrainingDummy> dummies,
                                    const combat::DamageRules& rules) {
    std::vector<HitEvent> hits;                          // 이번 박자에 맞은 일들
    for (auto& dummy : dummies) {                        // 허수아비를 하나씩 보며
        if (!dummy.can_be_hit()) continue;               // 쓰러져 있으면 건너뛰기
        const engine::RectF hurt = dummy.hurtbox();      // 허수아비의 맞는 상자
        if (!swing.hitbox.intersects(hurt)) continue;    // 칼이 안 닿으면 건너뛰기
        if (!swing.register_hit || !swing.register_hit(dummy.id())) continue;  // 이미 맞은 대상이면 건너뛰기

        combat::DamageInput input;                       // 피해 계산 재료
        input.attack_power = swing.attack_power;         // 공격력
        input.power = swing.power;                       // 위력 배수
        input.defense = dummy.defense();                 // 허수아비 방어력
        const auto result = combat::compute_damage(input, rules);  // 순수 함수로 계산
        if (!result.hit) continue;                       // 무적 등으로 안 맞았으면 건너뛰기

        const int applied = dummy.take_damage(result.amount);  // HP 깎기
        hits.push_back({dummy.id(), applied,
                        {hurt.x + hurt.w * 0.5f, hurt.y + hurt.h * 0.5f},  // 상자 가운데
                        dummy.down()});                  // 이번에 쓰러졌나
    }
    return hits;
}

}  // namespace wind::game
