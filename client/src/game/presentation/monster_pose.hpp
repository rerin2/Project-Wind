#pragma once

// 이 파일은 무엇을 하나요?
// 슬라임 몸 모양(눌림·늘어남·뜀 높이)을 계산하는 도우미 함수예요. (11a, 보여 주기 전용)
// - 쉴 때: 천천히 숨쉬듯 옆으로 퍼졌다 위로 늘어났다 해요.
// - 걸을 때: 걸은 거리에 맞춰 통통 튀어요. 공중에서는 위로 길쭉, 땅에 닿을 때는 납작.
// 그림 한 장을 늘이고 줄여서 움직임을 만들어요 (그림 칸을 많이 그리지 않아도 돼요).
// 게임 결과(위치·판정)는 바꾸지 않아요. 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

namespace wind::game {

// 그릴 때 쓰는 몸 모양
struct MonsterPose {
    float scale_x = 1.0f;  // 가로 배율 (1 = 그대로)
    float scale_y = 1.0f;  // 세로 배율
    float lift = 0.0f;     // 땅에서 뜬 높이 (픽셀, 그림자는 땅에 남아요)
};

// clock: 연출 시계 (초), phase: 마리마다 다른 숨쉬기 엇갈림 (0~1)
// distance: 걸은 거리 합 (픽셀), hop_blend: 뜀 세기 0~1 (걷기 시작하면 1로, 멈추면 0으로 부드럽게)
[[nodiscard]] MonsterPose slime_pose(float clock, float phase, float distance, float hop_blend);

// 뜀 세기를 목표(걷는 중 1, 아니면 0)로 부드럽게 옮겨요. 멈출 때 몸이 공중에서 뚝 떨어지지 않게요.
[[nodiscard]] float approach_hop_blend(float blend, bool walking, float dt_seconds);

}  // namespace wind::game
