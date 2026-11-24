// 이 파일은 무엇을 하나요?
// 필드·진행 상태 → HUD 값 옮겨 담기. 위에서부터 HP·MP·스태미나 바, 그다음 알림·룬.

#include "app/hud_info_source.hpp"

#include "game/progression/hero_progress.hpp"
#include "game/states/field_state.hpp"

namespace wind::client {

HudInfo collect_hud_info(const game::FieldState& field, const game::HeroProgress& progress) {
    HudInfo hud;
    hud.hp = field.health().fraction();  // 0~1과 최대값(바 길이용)
    hud.hp_max = static_cast<float>(field.health().max());
    hud.stamina = field.stamina().fraction();
    hud.stamina_max = field.stamina().max();
    hud.exhausted = field.stamina().exhausted();
    hud.mana = field.mana().fraction();
    hud.mana_max = field.mana().max();
    hud.mana_denied = field.mana_denied_count();  // 늘면 MP 바가 깜빡여요
    hud.fallen = field.hero_down();                // 쓰러지면 "쓰러졌다" 화면 (12b), 부활 안내 (12c)
    hud.rest_prompt = field.rest_prompt();         // 쿠션 가까이면 "[E] 쉬기" (13b)
    hud.rest_denied = field.rest_denied_count();   // 못 쉬면 "주변이 너무 소란스럽다" (13b)
    hud.runes = progress.runes();                  // 오른쪽 아래 룬 숫자 (13c)
    hud.lost_runes = progress.last_lost();         // "쓰러졌다" 화면의 잃은 룬 (13c)
    hud.vanished_runes = progress.last_vanished();  // 사라진 흔적 (14e)
    return hud;
}

}  // namespace wind::client
