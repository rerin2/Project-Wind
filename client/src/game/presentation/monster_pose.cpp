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
// 싸움 동작 (11b)
constexpr float kNoticeJumpPart = 0.5f;    // 알아채기 앞쪽 절반 동안 폴짝
constexpr float kNoticeJumpHeight = 4.0f;  // 폴짝 높이 (픽셀)
constexpr float kWindupSpread = 0.08f;     // 예고 끝에 옆으로 8% 더 퍼져요
constexpr float kWindupSquash = 0.10f;     // 예고 끝에 10% 더 낮아져요
constexpr float kShiverStart = 0.6f;       // 예고 60%부터 부르르
constexpr float kShiverHz = 15.0f;         // 떨림 빠르기 (1초에 15번 왕복)
constexpr float kLungeHeight = 7.0f;       // 덮칠 때 가장 높이 (픽셀)
constexpr float kSplatPart = 0.3f;         // 후딜 앞 30% 동안 납작
constexpr float kSplat = 0.18f;            // 땅에 떨어질 때 납작한 정도 (18%)
// 맞기·쓰러짐 (11d)
constexpr float kHurtSqueeze = 0.16f;      // 맞은 순간 16% 홀쭉·길쭉
constexpr float kMeltSpread = 0.35f;       // 쓰러질 때 옆으로 35% 퍼져요
constexpr float kMeltFlatten = 0.65f;      // 쓰러질 때 65% 낮아져요

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

std::optional<MonsterPose> slime_action_pose(MonsterState state, float progress, float clock) {
    const float p = std::clamp(progress, 0.0f, 1.0f);
    const float pi = std::numbers::pi_v<float>;
    MonsterPose pose;
    switch (state) {
        case MonsterState::Notice: {
            // 앞쪽 절반 동안 한 번 폴짝 (깜짝!), 나머지는 가만히
            const float jump = std::sin(pi * std::min(p / kNoticeJumpPart, 1.0f));
            pose.lift = jump * kNoticeJumpHeight;
            pose.scale_y = 1.0f + jump * kAirStretch * 0.5f;
            pose.scale_x = 1.0f - jump * kAirStretch * 0.25f;
            return pose;
        }
        case MonsterState::Windup: {
            // 웅크린 그림 + 점점 더 납작하게. 끝 무렵에는 부르르 떨어서 "곧 온다"가 보여요
            pose.frame = 1;
            pose.scale_x = 1.0f + p * kWindupSpread;
            pose.scale_y = 1.0f - p * kWindupSquash;
            if (p >= kShiverStart) {  // 1픽셀 왼쪽·오른쪽을 번갈아 (사인파 부호)
                pose.offset_x = std::sin(kTwoPi * kShiverHz * clock) >= 0.0f ? 1.0f : -1.0f;
            }
            return pose;
        }
        case MonsterState::Attack: {
            // 덮치기: 높이 뛰어 공중에서 길쭉
            const float air = std::sin(pi * p);
            pose.lift = air * kLungeHeight;
            pose.scale_y = 1.0f + air * kAirStretch;
            pose.scale_x = 1.0f - air * kAirStretch * 0.5f;
            return pose;
        }
        case MonsterState::Recover: {
            // 후딜 앞부분: 철퍽 납작했다가 원래대로
            const float splat = 1.0f - std::min(p / kSplatPart, 1.0f);
            pose.scale_x = 1.0f + splat * kSplat;
            pose.scale_y = 1.0f - splat * kSplat;
            return pose;
        }
        case MonsterState::Hurt: {
            // 맞아서 움찔 (11d): 홀쭉하고 키가 커졌다가 원래대로 (출렁)
            const float recoil = 1.0f - p;
            pose.scale_x = 1.0f - recoil * kHurtSqueeze;
            pose.scale_y = 1.0f + recoil * kHurtSqueeze;
            return pose;
        }
        case MonsterState::Dead: {
            // 쓰러짐 (11d): 옆으로 퍼지며 바닥에 녹아내리듯 납작해져요
            const float melt = p * p;  // 처음엔 천천히, 끝으로 갈수록 빨리
            pose.scale_x = 1.0f + melt * kMeltSpread;
            pose.scale_y = 1.0f - melt * kMeltFlatten;
            return pose;
        }
        default:
            return std::nullopt;  // 쉬기·걷기는 slime_pose
    }
}

bool hops_while(MonsterState state) {
    return state == MonsterState::Wander || state == MonsterState::Chase || state == MonsterState::Return;
}

float approach_hop_blend(float blend, bool walking, float dt_seconds) {
    const float target = walking ? 1.0f : 0.0f;           // 걷는 중이면 1, 아니면 0
    const float step = kHopBlendRate * std::max(dt_seconds, 0.0f);
    if (blend < target) return std::min(target, blend + step);  // 목표까지 일정하게 올리고
    return std::max(target, blend - step);                      // 내려요
}

}  // namespace wind::game
