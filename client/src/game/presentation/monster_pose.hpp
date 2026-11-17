#pragma once

// 이 파일은 무엇을 하나요?
// 슬라임 몸 모양(눌림·늘어남·뜀 높이·떨림)을 계산하는 도우미 함수예요. (11a, 11b, 보여 주기 전용)
// - 쉴 때: 천천히 숨쉬듯 옆으로 퍼졌다 위로 늘어났다 해요.
// - 걸을 때(배회·쫓기·복귀): 걸은 거리에 맞춰 통통 튀어요. 공중에서는 위로 길쭉, 땅에 닿을 때는 납작.
// - 알아챌 때: 깜짝 놀라 한 번 폴짝 (11b)
// - 공격 예고: 웅크린 그림(1번 칸)으로 점점 납작해지고, 끝 무렵 부르르 떨어요 (11b, 번쩍임은 안 써요)
// - 덮칠 때: 높이 뛰어오르며 길쭉해지고, 후딜 처음에 철퍽 납작해져요 (11b)
// 그림 한 장을 늘이고 줄여서 움직임을 만들어요. 게임 결과(위치·판정)는 바꾸지 않아요.
// 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

#include "game/monsters/monster_state.hpp"

#include <optional>

namespace wind::game {

// 그릴 때 쓰는 몸 모양
struct MonsterPose {
    float scale_x = 1.0f;   // 가로 배율 (1 = 그대로)
    float scale_y = 1.0f;   // 세로 배율
    float lift = 0.0f;      // 땅에서 뜬 높이 (픽셀, 그림자는 땅에 남아요)
    float offset_x = 0.0f;  // 옆으로 떨리는 정도 (픽셀, 예고 떨림)
    int frame = 0;          // 그림 칸: 0 = 보통, 1 = 웅크림 (예고)
};

// clock: 연출 시계 (초), phase: 마리마다 다른 숨쉬기 엇갈림 (0~1)
// distance: 걸은 거리 합 (픽셀), hop_blend: 뜀 세기 0~1 (걷기 시작하면 1로, 멈추면 0으로 부드럽게)
[[nodiscard]] MonsterPose slime_pose(float clock, float phase, float distance, float hop_blend);

// 싸움 동작(알아채기·예고·덮치기·후딜)의 모양. 다른 상태면 nullopt (그때는 slime_pose를 써요).
// progress: 그 상태의 진행 0~1, clock: 떨림용 연출 시계 (초)
[[nodiscard]] std::optional<MonsterPose> slime_action_pose(MonsterState state, float progress, float clock);

// 이 상태에서 통통 뛰며 걷나요? (배회·쫓기·복귀)
[[nodiscard]] bool hops_while(MonsterState state);

// 뜀 세기를 목표(걷는 중 1, 아니면 0)로 부드럽게 옮겨요. 멈출 때 몸이 공중에서 뚝 떨어지지 않게요.
[[nodiscard]] float approach_hop_blend(float blend, bool walking, float dt_seconds);

}  // namespace wind::game
