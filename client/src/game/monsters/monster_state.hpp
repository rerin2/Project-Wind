#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터가 지금 무엇을 하는지(상태) 이름표예요. (명세 9.6)
// 11a: 대기·배회. 11b: 알아채기·쫓기·공격 예고·공격·후딜·복귀. 11d: 맞아서 움찔·쓰러짐.
// 몬스터 그림(MonsterView)과 F3 정보판도 이 이름표를 봐서 따로 파일로 뒀어요.

#include <cstdint>
#include <string_view>

namespace wind::game {

enum class MonsterState : std::uint8_t {
    Idle,     // 제자리 근처에서 잠깐 쉬어요
    Wander,   // 제자리 둘레의 한 점으로 천천히 걸어가요
    Notice,   // 주인공을 알아챘어요! 느낌표를 띄우고 잠깐 멈춰요
    Chase,    // 주인공 쪽으로 쫓아가요
    Windup,   // 공격 예고: 멈춰서 웅크리고, 바닥에 덮칠 자리가 붉게 보여요
    Attack,   // 예고 때 정한 방향으로 덮쳐요 (이때만 공격 판정이 있어요)
    Recover,  // 덮친 뒤 빈틈 (후딜)
    Hurt,     // 칼·마술에 맞아 움찔 (경직, 11d). 끝나면 주인공을 쫓아요
    Return,   // 포기하고 제자리로 돌아가요
    Dead,     // HP 0: 쓰러져서 잠깐 뒤 사라져요 (11d). 아무것도 안 하고 맞지도 않아요
};

// F3 정보판·로그용 영어 이름
[[nodiscard]] constexpr std::string_view to_string(MonsterState state) {
    switch (state) {
        case MonsterState::Idle: return "idle";
        case MonsterState::Wander: return "wander";
        case MonsterState::Notice: return "notice";
        case MonsterState::Chase: return "chase";
        case MonsterState::Windup: return "windup";
        case MonsterState::Attack: return "attack";
        case MonsterState::Recover: return "recover";
        case MonsterState::Hurt: return "hurt";
        case MonsterState::Return: return "return";
        case MonsterState::Dead: return "dead";
    }
    return "?";
}

}  // namespace wind::game
