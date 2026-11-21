// 이 파일은 무엇을 하나요?
// 쓰러짐 화면이 다 나타난 뒤의 시간을 세고, Enter나 2.5초가 되면 부활을 알리는 곳이에요.

#include "game/world/respawn_trigger.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

bool RespawnTrigger::update(float dt_seconds, bool hero_down, bool screen_ready, bool confirm) {
    if (!hero_down || !screen_ready) {
        waited_ = 0.0f;  // 서 있거나 아직 화면이 다 안 나타났으면 처음부터
        return false;
    }
    if (std::isfinite(dt_seconds) && dt_seconds > 0.0f) {
        waited_ += dt_seconds;
    }
    if (!confirm && waited_ < kAutoSeconds) {
        return false;  // 아직 기다려요
    }
    waited_ = 0.0f;  // 알렸어요. 그래도 쓰러져 있으면(맵을 못 엶) 2.5초 뒤 다시 알려요
    return true;
}

float RespawnTrigger::auto_left() const {
    return std::max(0.0f, kAutoSeconds - waited_);
}

}  // namespace wind::game
