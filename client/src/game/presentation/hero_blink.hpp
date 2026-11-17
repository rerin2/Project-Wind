#pragma once

// 이 파일은 무엇을 하나요?
// 맞은 뒤 무적인 동안 주인공을 깜빡이게 할 투명도를 정하는 도우미 함수예요. (명세 6.7, 9.4, 11c)
// 0.1초마다 한 번씩 흐려졌다(25%) 돌아와요. 완전히 사라지지는 않아서 어디 있는지 늘 보여요.
// 구르기 무적은 깜빡이지 않아요 (맞은 뒤 무적만). 보여 주기만 해요.

#include "game/entities/hit_reaction.hpp"

namespace wind::game {

// 주인공 그림의 투명도 0~1 (무적이 아니면 1)
[[nodiscard]] float hero_blink_alpha(const HitReaction& reaction);

}  // namespace wind::game
