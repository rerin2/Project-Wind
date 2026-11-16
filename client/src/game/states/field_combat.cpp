// 이 파일은 무엇을 하나요?
// 필드의 칼 판정과 허수아비 시간을 박자마다 진행하는 곳이에요.

#include "game/states/field_combat.hpp"

#include "wind/log.hpp"

#include <utility>

namespace wind::game {
namespace {

// 힘 보너스 (명세 8.2). 능력치가 생기는 13단계 전까지는 시작 힘 10의 보너스 0이에요.
constexpr float kStrengthBonus = 0.0f;

}  // namespace

FieldCombat::FieldCombat(std::vector<TrainingDummy> dummies) : dummies_(std::move(dummies)) {}

void FieldCombat::step(double dt_seconds, PlayerMotion& player) {
    hits_.clear();                                              // 지난 박자 기록 지우기
    const auto hitbox = player.attack_hitbox();                 // 판정 중이 아니면 비어 있어요
    const auto* attack = player.attack().current();             // 지금 휘두르는 공격
    if (hitbox && attack != nullptr && !dummies_.empty()) {     // 칼이 판정 중이고 대상이 있으면
        const auto& weapon = player.attack().weapon();          // 든 무기
        Swing swing;                                            // 이번 칼 한 번
        swing.hitbox = *hitbox;                                 // 닿는 곳
        swing.attack_power = combat::physical_attack_power(weapon.base_power, weapon.strength_scaling,
                                                           kStrengthBonus);  // 공격력
        swing.power = attack->power;                            // 콤보마다 다른 위력
        swing.register_hit = [&player](TargetId id) { return player.register_hit(id); };  // 한 번만 맞게
        hits_ = resolve_swing(swing, dummies_);                 // 맞았는지 보고 피해 주기
        for (const auto& hit : hits_) {                         // 맞은 일을 기록해요
            last_hit_ = hit;
            log::debug("Hit dummy {} for {}{}", hit.target, hit.amount,
                       hit.knocked_down ? " (knocked down)" : "");
        }
    }
    for (auto& dummy : dummies_) {                              // 쓰러진 허수아비는 다시 서기까지 시간 세기
        dummy.step(static_cast<float>(dt_seconds));
    }
}

std::string FieldCombat::debug_text() const {
    std::string text = "dummies " + std::to_string(dummies_.size());          // 허수아비 수
    if (!last_hit_) return text;                                              // 아직 아무도 안 맞았으면 여기까지
    text += "  last hit " + std::to_string(last_hit_->amount) + " -> #" + std::to_string(last_hit_->target);
    const auto index = static_cast<std::size_t>(last_hit_->target);           // 대상 번호는 1부터
    if (index >= 1 && index <= dummies_.size()) {                           // 그 허수아비의 지금 HP
        const auto& dummy = dummies_[index - 1];
        text += " " + std::to_string(dummy.hp()) + "/" + std::to_string(dummy.max_hp());
        if (dummy.down()) text += " (down)";                                  // 쓰러져 있으면 표시
    }
    return text;
}

}  // namespace wind::game
