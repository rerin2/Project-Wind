#pragma once

// 이 파일은 무엇을 하나요?
// 공격 하나(예: 롱소드 첫 번째 휘두르기)를 적은 데이터예요. (명세 9.2)
// 휘두르기는 세 구간으로 나뉘어요.
//   준비(windup) -> 판정(active, 이때만 맞아요) -> 후딜(recovery, 빈틈)
// 숫자는 content/attacks/*.json 파일에서 읽어요. 코드에 숫자를 박아 두지 않아요.

#include <optional>
#include <string>

namespace wind::combat {

// 공격이 닿는 상자 (히트박스). 주인공이 "오른쪽을 볼 때" 기준으로 적어요.
// forward: 발에서 앞으로 얼마나 떨어졌는지 (상자 가운데, 픽셀)
// lift:    화면에서 위아래로 얼마나 옮길지 (음수 = 위, 몸통 높이를 맞출 때 써요)
// length:  앞뒤 길이, width: 옆 폭 (픽셀)
// 다른 방향을 볼 때 상자를 어떻게 돌리는지는 docs/ENGINE.md "Attack" 절에 적어 둬요.
struct HitboxShape {
    float forward = 0.0f;
    float lift = 0.0f;
    float length = 1.0f;
    float width = 1.0f;
};

struct AttackData {
    std::string id;                // "longsword_1"
    float windup = 0.0f;           // 준비 시간 (초)
    float active = 0.0f;           // 판정 시간 (초). 이 동안 히트박스에 닿으면 맞아요
    float recovery = 0.0f;         // 후딜 시간 (초)
    float cancel_start = 0.0f;     // 취소 가능 구간 시작 (공격 시작부터 초)
    float cancel_end = 0.0f;       // 취소 가능 구간 끝
    HitboxShape hitbox;            // 공격이 닿는 상자
    float power = 1.0f;            // 위력 배수 (피해 = 공격력 × 위력, 9.3)
    float knockback = 0.0f;        // 밀어내는 힘 (픽셀/초)
    float hitstop = 0.0f;          // 맞는 순간 멈칫하는 시간 (초, 연출)
    float shake = 0.0f;            // 화면 흔들림 세기 0~1 (연출)
    int mp_cost = 0;               // 시작할 때 쓰는 MP (보통 0)
    std::optional<std::string> fx;          // 맞을 때 효과 파일 (선택)
    std::optional<std::string> sfx;         // 맞을 때 소리 파일 (선택)
    std::optional<std::string> next_combo;  // 다음 콤보 공격 id (선택, 무기 목록 순서와 같아야 해요)

    // 처음부터 끝까지 걸리는 시간
    [[nodiscard]] float total_seconds() const { return windup + active + recovery; }
};

}  // namespace wind::combat
