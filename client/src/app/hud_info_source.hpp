#pragma once

// 이 파일은 무엇을 하나요?
// HUD가 보여 줄 값(HP·MP·스태미나, 쓰러짐, 쉬기 안내, 룬)을 필드와 진행 상태에서 모아요.
// client_app.cpp가 300줄을 넘지 않게 게임 루프에서 빼냈어요 (15e). 계산은 하지 않고 옮겨 담기만 해요.

#include "app/field_ui.hpp"

namespace wind::game {
class FieldState;
class HeroProgress;
}  // namespace wind::game

namespace wind::client {

[[nodiscard]] HudInfo collect_hud_info(const game::FieldState& field, const game::HeroProgress& progress);

}  // namespace wind::client
