// 이 파일은 무엇을 하나요?
// 공격 한 번이 허수아비들에 맞았는지 보고, 피해를 주고, 밀어내는 곳이에요.

#include "game/combat/hit_resolution.hpp"

namespace wind::game {

std::vector<HitEvent> resolve_swing(const Swing& swing, std::span<TrainingDummy> dummies,
                                    CombatRandom* random, const combat::DamageRules& rules) {
    std::vector<HitEvent> hits;                          // 이번 박자에 맞은 일들
    for (auto& dummy : dummies) {                        // 허수아비를 하나씩 보며
        if (!dummy.can_be_hit()) continue;               // 쓰러져 있으면 건너뛰기
        const engine::RectF hurt = dummy.hurtbox();      // 허수아비의 맞는 상자
        if (!swing.hitbox.intersects(hurt)) continue;    // 공격이 안 닿으면 건너뛰기
        if (!swing.register_hit || !swing.register_hit(dummy.id())) continue;  // 이미 맞은 대상이면 건너뛰기

        combat::DamageInput input;                       // 피해 계산 재료
        input.attack_power = swing.attack_power;         // 공격력
        input.power = swing.power;                       // 위력 배수
        input.defense = dummy.defense();                 // 허수아비 방어력
        input.critical = random != nullptr && random->roll(swing.crit_chance);  // 치명타 주사위
        const auto result = combat::compute_damage(input, rules);  // 순수 함수로 계산
        if (!result.hit) continue;                       // 무적 등으로 안 맞았으면 건너뛰기

        const int applied = dummy.take_damage(result.amount);  // HP 깎기
        dummy.push(swing.push_direction, swing.knockback);     // 밀어내기 (쓰러지는 마지막 한 대도 밀려요)
        HitEvent hit;
        hit.target = dummy.id();
        hit.amount = applied;
        hit.at = {hurt.x + hurt.w * 0.5f, hurt.y + hurt.h * 0.5f};  // 맞기 전 상자 가운데
        hit.knocked_down = dummy.down();                 // 이번에 쓰러졌나
        hit.critical = input.critical;
        hit.direction = swing.push_direction;
        hit.knockback = swing.knockback;
        hit.hitstop = swing.hitstop;
        hit.shake = swing.shake;
        hits.push_back(hit);
    }
    return hits;
}

}  // namespace wind::game
