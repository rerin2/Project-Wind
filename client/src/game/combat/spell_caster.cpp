// 이 파일은 무엇을 하나요?
// 마술 시전 시간을 재고, 다 모은 박자에 "나갔다"를 알려 주는 곳이에요.

#include "game/combat/spell_caster.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {
namespace {

// 1/60초를 여러 번 더하면 0.25가 아주 조금 모자랄 수 있어서 작은 여유를 둬요.
// (여유가 없으면 15박자가 아니라 16박자째에 나가요)
constexpr float kTimeEpsilon = 1e-4f;

// 지금 움직일 수 있는 마술인가요? 15b까지는 빛 탄 하나를 똑바로 쏘는 마술만 (별빛 조각·큰 별빛 조각).
// 근접(달빛 칼날)·여러 개·유도(별무리)는 15c, 광선(혜성 광선)은 15d에서 열어요. 그 전에는 MP도 안 써요.
bool castable(const combat::Spell& spell) {
    if (spell.motion != combat::SpellMotion::Projectile || !spell.projectile) return false;
    const auto& shot = *spell.projectile;
    return shot.count == 1 && shot.homing == 0.0f && shot.orbit_seconds == 0.0f;
}

}  // namespace

SpellCaster::SpellCaster(std::optional<combat::Spell> spell) : spell_(std::move(spell)) {}

void SpellCaster::set_spell(std::optional<combat::Spell> spell) {
    cancel();  // 모으던 마술은 버려요
    spell_ = std::move(spell);
}

bool SpellCaster::can_start() const {
    return spell_ && castable(*spell_) && !casting_;
}

bool SpellCaster::start() {
    if (!can_start()) {
        return false;
    }
    casting_ = true;
    elapsed_ = 0.0f;
    return true;
}

void SpellCaster::cancel() {
    casting_ = false;
    elapsed_ = 0.0f;
}

bool SpellCaster::step(float dt_seconds) {
    if (!casting_ || dt_seconds < 0.0f) {
        return false;
    }
    elapsed_ += dt_seconds;
    if (elapsed_ + kTimeEpsilon < spell_->cast_seconds) {
        return false;  // 아직 모으는 중
    }
    casting_ = false;  // 다 모았어요: 마술이 나가요
    elapsed_ = 0.0f;
    return true;
}

float SpellCaster::progress() const {
    if (!casting_ || !spell_ || spell_->cast_seconds <= 0.0f) {
        return 0.0f;
    }
    return std::clamp(elapsed_ / spell_->cast_seconds, 0.0f, 1.0f);
}

float SpellCaster::mp_cost() const {
    return spell_ ? static_cast<float>(spell_->mp_cost) : 0.0f;
}

}  // namespace wind::game
