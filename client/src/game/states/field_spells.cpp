// 이 파일은 무엇을 하나요?
// 필드의 마술 담당: 고른 마술 쥐여 주기, Q로 다음 마술.

#include "game/states/field_spells.hpp"

#include "game/combat/spell_shelf.hpp"
#include "game/entities/player_motion.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/log.hpp"

namespace wind::game {

std::size_t FieldSpells::open_slots() const {
    return static_cast<std::size_t>(progress_.derived().spell_slots);  // 지능 곡선 (명세 8.2)
}

const combat::Spell* FieldSpells::selected() const {
    const auto id = progress_.spells().selected(open_slots());
    return id.empty() ? nullptr : shelf_.find(id);
}

void FieldSpells::arm(PlayerMotion& hero) const {
    const combat::Spell* spell = selected();
    hero.set_spell(spell != nullptr ? std::optional<combat::Spell>(*spell) : std::nullopt);
}

bool FieldSpells::step(PlayerMotion& hero) {
    if (!next_pressed_) return false;
    next_pressed_ = false;                  // 누름은 한 번만 써요
    if (hero.caster().busy()) return false;  // 시전 중 Q는 무시 (개발자와 정함)
    if (!progress_.spells().select_next(open_slots())) return false;  // 다른 마술이 없어요
    arm(hero);
    ++switch_count_;
    log::debug("Selected spell {}", selected() != nullptr ? selected()->id : std::string("(none)"));
    return true;
}

}  // namespace wind::game
