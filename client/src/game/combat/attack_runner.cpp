// 이 파일은 무엇을 하나요?
// 공격 시간 흐름과 콤보 순서, "한 번 휘두를 때 한 번만 맞기"를 처리해요.

#include "game/combat/attack_runner.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {

AttackRunner::AttackRunner(combat::Weapon weapon) : weapon_(std::move(weapon)) {}

const combat::AttackData* AttackRunner::current() const {
    if (index_ < 0 || static_cast<std::size_t>(index_) >= weapon_.combo.size()) {
        return nullptr;
    }
    return &weapon_.combo[static_cast<std::size_t>(index_)];
}

AttackRunner::Phase AttackRunner::phase() const {
    const auto* attack = current();
    if (attack == nullptr) {
        return Phase::None;
    }
    if (elapsed_ < attack->windup) {
        return Phase::Windup;
    }
    if (elapsed_ < attack->windup + attack->active) {
        return Phase::Active;
    }
    return Phase::Recovery;
}

bool AttackRunner::in_cancel_window() const {
    const auto* attack = current();
    return attack != nullptr && elapsed_ >= attack->cancel_start && elapsed_ <= attack->cancel_end;
}

bool AttackRunner::can_start() const {
    if (!has_weapon()) {
        return false;
    }
    if (!busy()) {
        return true;
    }
    const bool has_next = static_cast<std::size_t>(index_ + 1) < weapon_.combo.size();
    return has_next && in_cancel_window();
}

bool AttackRunner::start() {
    if (!can_start()) {
        return false;
    }
    index_ = busy() ? index_ + 1 : 0;
    elapsed_ = 0.0f;
    hit_targets_.clear();  // 새 휘두르기: 맞힌 목록을 비워요
    return true;
}

void AttackRunner::cancel() {
    index_ = -1;
    elapsed_ = 0.0f;
    hit_targets_.clear();
}

void AttackRunner::step(float dt_seconds) {
    const auto* attack = current();
    if (attack == nullptr) {
        return;
    }
    elapsed_ += std::max(0.0f, dt_seconds);
    if (elapsed_ >= attack->total_seconds()) {
        cancel();  // 끝까지 휘둘렀어요. 다음 공격은 1타부터.
    }
}

bool AttackRunner::register_hit(TargetId target) {
    if (phase() != Phase::Active) {
        return false;
    }
    if (std::ranges::find(hit_targets_, target) != hit_targets_.end()) {
        return false;
    }
    hit_targets_.push_back(target);
    return true;
}

}  // namespace wind::game
