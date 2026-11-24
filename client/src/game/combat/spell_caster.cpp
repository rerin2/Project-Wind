// 이 파일은 무엇을 하나요?
// 마술 시전 시간을 재고, 다 모은 박자에 "나갔다"를 알리거나 광선 유지로 넘어가는 곳이에요.

#include "game/combat/spell_caster.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {
namespace {

// 1/60초를 여러 번 더하면 0.25가 아주 조금 모자랄 수 있어서 작은 여유를 둬요.
// (여유가 없으면 15박자가 아니라 16박자째에 나가요)
constexpr float kTimeEpsilon = 1e-4f;

// 지금 움직일 수 있는 마술인가요? 동작에 맞는 데이터가 있어야 해요.
// 빛 탄(별무리 포함, 15c), 근접(달빛 칼날, 15c), 광선(혜성 광선, 15d)
bool castable(const combat::Spell& spell) {
    switch (spell.motion) {
        case combat::SpellMotion::Projectile: return spell.projectile.has_value();
        case combat::SpellMotion::Melee: return spell.melee.has_value();
        case combat::SpellMotion::Beam: return spell.beam.has_value();
    }
    return false;
}

}  // namespace

SpellCaster::SpellCaster(std::optional<combat::Spell> spell) : spell_(std::move(spell)) {}

void SpellCaster::set_spell(std::optional<combat::Spell> spell) {
    cancel();  // 모으던 마술·쏘던 광선은 버려요
    spell_ = std::move(spell);
}

bool SpellCaster::can_start() const {
    return spell_ && castable(*spell_) && !busy();
}

bool SpellCaster::start() {
    if (!can_start()) {
        return false;
    }
    phase_ = Phase::Charging;
    elapsed_ = 0.0f;
    return true;
}

void SpellCaster::cancel() {
    phase_ = Phase::Idle;
    elapsed_ = 0.0f;
}

bool SpellCaster::step(float dt_seconds, bool held) {
    if (!busy() || dt_seconds < 0.0f) {
        return false;
    }
    const bool beam = spell_->motion == combat::SpellMotion::Beam;
    if (beam && !held) {  // 광선은 버튼을 떼면 끝 (모으는 중이면 광선이 안 나가요)
        cancel();
        return false;
    }
    if (phase_ == Phase::Beaming) {
        return false;  // 광선 유지 중: 누르고 있는 한 계속돼요
    }
    elapsed_ += dt_seconds;
    if (elapsed_ + kTimeEpsilon < spell_->cast_seconds) {
        return false;  // 아직 모으는 중
    }
    elapsed_ = 0.0f;
    if (beam) {
        phase_ = Phase::Beaming;  // 다 모았어요: 광선을 쏘기 시작해요
        return false;
    }
    phase_ = Phase::Idle;  // 다 모았어요: 마술이 나가요
    return true;
}

float SpellCaster::progress() const {
    if (phase_ != Phase::Charging || !spell_ || spell_->cast_seconds <= 0.0f) {
        return 0.0f;
    }
    return std::clamp(elapsed_ / spell_->cast_seconds, 0.0f, 1.0f);
}

float SpellCaster::mp_cost() const {
    if (!spell_ || spell_->motion == combat::SpellMotion::Beam) return 0.0f;  // 광선은 쏘는 동안 내요
    return static_cast<float>(spell_->mp_cost);
}

float SpellCaster::mp_to_start() const {
    return spell_ ? combat::start_mp(*spell_) : 0.0f;
}

}  // namespace wind::game
