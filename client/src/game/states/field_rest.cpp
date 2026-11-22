// 이 파일은 무엇을 하나요?
// 쿠션 쉬기 흐름: E 처리, 쉬기 조건, 앉은 뒤 쉬기 이동 요청.

#include "game/states/field_rest.hpp"

#include "game/entities/player_motion.hpp"
#include "game/states/field_combat.hpp"

#include "wind/log.hpp"

#include <utility>

namespace wind::game {

FieldRest::FieldRest(std::vector<Cushion> cushions, RestRules rules)
    : cushions_(std::move(cushions)), rules_(rules) {}

void FieldRest::step(float dt_seconds, PlayerMotion& player, const FieldCombat& combat) {
    if (std::exchange(interact_, false)) interact(player, combat);  // 이번 박자의 E (한 번만)
    if (!player.rest().active()) resting_on_.reset();              // 다 일어났으면 쿠션에서 내려와요
    if (travel_wait_ < 0.0f || !resting_on_) return;               // 쉬기 이동을 기다리는 중이 아니에요
    travel_wait_ -= dt_seconds;                                    // 히트스톱(0)이면 기다림도 멈춰요
    if (travel_wait_ <= 0.0f) {
        travel_wait_ = -1.0f;
        travel_request_ = cushions_[*resting_on_].id;              // 앱에게: 페이드하고 새로 읽어 주세요
    }
}

void FieldRest::interact(PlayerMotion& player, const FieldCombat& combat) {
    if (player.rest().active()) {   // 앉아 있으면 E = 일어나기 (13b 임시, 13c부터는 메뉴)
        (void)player.stand_up();
        return;
    }
    if (player.dead()) return;      // 쓰러져 있으면 아무 일도 없어요
    const RestDecision decision =
        check_rest(player.mover().feet(), cushions_, combat.monster_engaged(), player.seconds_since_hit(), rules_);
    if (decision.result == RestCheck::NoCushion) return;  // 쿠션 근처가 아니에요 (E는 다른 일에 쓰일 수 있어요)
    if (decision.result == RestCheck::TooNoisy) {
        ++denied_count_;                                   // "주변이 너무 소란스럽다"
        log::debug("Cannot rest: monster engaged or hit {:.1f} s ago", player.seconds_since_hit());
        return;
    }
    const Cushion& cushion = cushions_[*decision.cushion];
    if (!player.begin_rest(cushion.seat)) return;          // 공격·구르기 중이면 못 앉아요
    resting_on_ = decision.cushion;
    travel_wait_ = kRestTravelDelaySeconds;                // 잠깐 앉아 있다가 페이드
    log::info("Resting at cushion '{}'", cushion.id);
}

bool FieldRest::sit_on(std::string_view cushion_id, PlayerMotion& player) {
    for (std::size_t i = 0; i < cushions_.size(); ++i) {
        if (cushions_[i].id != cushion_id) continue;
        player.sit_on(cushions_[i].seat);
        resting_on_ = i;      // 이미 쉬었으니(쉬기 이동 뒤) 다시 이동을 요청하지 않아요
        travel_wait_ = -1.0f;
        return true;
    }
    return false;
}

std::optional<std::string> FieldRest::take_travel_request() {
    return std::exchange(travel_request_, std::nullopt);
}

const Cushion* FieldRest::prompt_cushion(const PlayerMotion& player) const {
    if (player.dead() || player.rest().active()) return nullptr;  // 쓰러졌거나 이미 쉬는 중
    const auto index = nearest_cushion(player.mover().feet(), cushions_, rules_);
    return index ? &cushions_[*index] : nullptr;
}

std::string FieldRest::name_of(std::string_view cushion_id) const {
    for (const auto& cushion : cushions_) {
        if (cushion.id == cushion_id) return cushion.name;
    }
    return {};  // 이 맵에 없는 쿠션
}

const Cushion* FieldRest::resting_cushion() const {
    return resting_on_ ? &cushions_[*resting_on_] : nullptr;
}

}  // namespace wind::game
