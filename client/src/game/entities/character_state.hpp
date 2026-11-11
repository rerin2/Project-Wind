#pragma once

// 이 파일은 무엇을 하나요?
// 캐릭터가 "지금 무엇을 하는 중인지"의 목록과, 어떤 상태에서 어떤 상태로 넘어갈 수 있는지 적은 표예요.
// (명세 8.5) 예: 쓰러진(dead) 상태에서는 공격이나 이동을 할 수 없어요.
//
// 표는 "넘어갈 수 있는 길이 있나"만 알려 줘요. 길에 붙은 조건(예: 공격 중 구르기는 취소 구간에서만)은
// 그 상태를 다루는 코드(AttackRunner, PlayerMotion)가 따로 확인해요.
// 지침(exhausted)은 상태가 아니라 스태미나의 표시(Stamina::exhausted)예요. 지쳐도 걷기는 되거든요.

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace wind::game {

enum class CharacterState : std::uint8_t {
    Idle,           // 가만히 서 있음
    Moving,         // 걷는 중
    Running,        // 달리는 중 (Shift)
    Attacking,      // 무기를 휘두르는 중
    Casting,        // 마술을 쓰는 중
    Dodging,        // 구르는 중
    Hit,            // 맞아서 움찔 (경직)
    Resting,        // 세이브 포인트에서 쉬는 중 (메뉴)
    Dead,           // 쓰러짐
    Transitioning,  // 다른 맵으로 가는 중
    InDialogue,     // 대화 중
    Count           // 개수 세기용 (진짜 상태 아님)
};

inline constexpr std::size_t kCharacterStateCount = static_cast<std::size_t>(CharacterState::Count);

// from에서 to로 넘어갈 수 있나요? 같은 상태 그대로(from == to)는 언제나 true예요.
// 단, Attacking -> Attacking은 "다음 콤보로 넘어가기"라는 뜻이라 표에 따로 적혀 있어요.
[[nodiscard]] bool can_transition(CharacterState from, CharacterState to);

// 상태 이름 (디버그 화면용). 예: "attacking"
[[nodiscard]] std::string_view to_string(CharacterState state);

}  // namespace wind::game
