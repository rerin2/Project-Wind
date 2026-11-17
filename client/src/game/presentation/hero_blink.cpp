// 이 파일은 무엇을 하나요?
// 무적 남은 시간으로 주인공 깜빡임 투명도를 계산하는 곳이에요.

#include "game/presentation/hero_blink.hpp"

#include <cmath>

namespace wind::game {
namespace {

constexpr float kBlinkPeriod = 0.1f;  // 한 번 깜빡이는 시간 (초): 흐림 0.05초 + 보임 0.05초
constexpr float kDimAlpha = 0.25f;    // 흐릴 때 투명도

}  // namespace

float hero_blink_alpha(const HitReaction& reaction) {
    if (!reaction.invulnerable()) return 1.0f;  // 무적이 아니면 그대로
    // 남은 시간을 박자로 나눠 앞쪽 절반은 흐리게: 맞은 순간(남은 시간이 가장 클 때)부터 흐려져요
    const float phase = std::fmod(reaction.invulnerable_left(), kBlinkPeriod) / kBlinkPeriod;
    return phase >= 0.5f ? kDimAlpha : 1.0f;
}

}  // namespace wind::game
