// 이 파일은 무엇을 하나요?
// 슬라임 숨쉬기와 통통 뛰기 모양을 계산하는 곳이에요. 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

#include "game/presentation/monster_pose.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

constexpr float kTwoPi = 2.0f * std::numbers::pi_v<float>;
constexpr float kBreathSeconds = 1.4f;   // 숨 한 번 (초)
constexpr float kBreathAmount = 0.06f;   // 숨쉴 때 늘어나고 줄어드는 정도 (6%)
constexpr float kHopLength = 14.0f;      // 한 번 뛸 때 가는 거리 (픽셀)
constexpr float kHopHeight = 3.0f;       // 가장 높이 뜰 때 (픽셀)
constexpr float kAirStretch = 0.16f;     // 공중에서 위로 길쭉해지는 정도
constexpr float kLandSquash = 0.10f;     // 땅에 닿을 때 납작해지는 정도
constexpr float kHopBlendRate = 6.0f;    // 뜀 세기가 1초에 바뀌는 양 (약 0.17초면 다 바뀌어요)

}  // namespace

MonsterPose slime_pose(float clock, float phase, float distance, float hop_blend) {
    const float blend = std::clamp(hop_blend, 0.0f, 1.0f);
    // 1) 숨쉬기: 사인파 하나로 가로가 커지면 세로는 작아져요 (부피가 비슷해 보이게)
    const float breath = std::sin(kTwoPi * (clock / kBreathSeconds + phase)) * kBreathAmount * (1.0f - blend);
    // 2) 뛰기: 걸은 거리로 몇 번째 뜀의 어디쯤인지 (0 = 땅, 0.5 = 꼭대기, 1 = 다시 땅)
    const float hop_t = std::fmod(std::max(distance, 0.0f), kHopLength) / kHopLength;
    const float air = std::sin(std::numbers::pi_v<float> * hop_t) * blend;  // 공중 정도 0~1
    const float ground = blend - air;                                      // 땅 정도 (뛰지 않으면 0)
    MonsterPose pose;
    pose.scale_x = 1.0f + breath - air * kAirStretch * 0.5f + ground * kLandSquash;  // 공중 홀쭉, 땅 납작
    pose.scale_y = 1.0f - breath + air * kAirStretch - ground * kLandSquash;
    pose.lift = air * kHopHeight;
    return pose;
}

float approach_hop_blend(float blend, bool walking, float dt_seconds) {
    const float target = walking ? 1.0f : 0.0f;           // 걷는 중이면 1, 아니면 0
    const float step = kHopBlendRate * std::max(dt_seconds, 0.0f);
    if (blend < target) return std::min(target, blend + step);  // 목표까지 일정하게 올리고
    return std::max(target, blend - step);                      // 내려요
}

}  // namespace wind::game
