// 이 파일은 무엇을 하나요?
// 캐릭터 상태 전이 표. 표 내용은 docs/GAMEPLAY.md "Character states" 절과 같아요.

#include "game/entities/character_state.hpp"

#include <array>
#include <initializer_list>

namespace wind::game {
namespace {

using S = CharacterState;
using Row = std::array<bool, kCharacterStateCount>;
using Table = std::array<Row, kCharacterStateCount>;

constexpr std::size_t at(S s) {
    return static_cast<std::size_t>(s);
}

// from 줄에 "갈 수 있는 상태들"을 표시해요.
constexpr void allow(Table& table, S from, std::initializer_list<S> targets) {
    for (const S to : targets) {
        table[at(from)][at(to)] = true;
    }
}

constexpr Table make_table() {
    Table t{};
    // 땅에 서 있거나 움직이는 상태: 거의 모든 행동을 시작할 수 있어요.
    for (const S free : {S::Idle, S::Moving, S::Running}) {
        allow(t, free, {S::Idle, S::Moving, S::Running, S::Attacking, S::Casting, S::Dodging, S::Hit,
                        S::Resting, S::Dead, S::Transitioning, S::InDialogue});
    }
    // 공격 중: 끝나면 서기/걷기, 취소 구간에서 다음 콤보나 구르기, 맞거나 쓰러질 수 있어요.
    allow(t, S::Attacking, {S::Idle, S::Moving, S::Running, S::Attacking, S::Dodging, S::Hit, S::Dead});
    // 마술 중: 끝나면 서기/걷기, 시전 전이나 광선 유지 중에만 구르기(조건은 마술 코드가 확인)
    allow(t, S::Casting, {S::Idle, S::Moving, S::Running, S::Dodging, S::Hit, S::Dead});
    // 구르는 중: 끝나야 다른 행동. 무적이 끝난 꼬리에서는 맞을 수 있어요.
    allow(t, S::Dodging, {S::Idle, S::Moving, S::Running, S::Hit, S::Dead});
    // 맞아서 움찔: 경직이 끝나면 서기/걷기, 또 맞거나 쓰러질 수 있어요.
    allow(t, S::Hit, {S::Idle, S::Moving, S::Running, S::Hit, S::Dead});
    // 쉬는 중: 메뉴를 닫으면 서기. 쉬는 동안에는 게임 입력이 막혀요.
    allow(t, S::Resting, {S::Idle});
    // 쓰러짐: 부활(맵 이동 또는 그 자리에서 서기)만 가능
    allow(t, S::Dead, {S::Transitioning, S::Idle});
    allow(t, S::Transitioning, {S::Idle});
    allow(t, S::InDialogue, {S::Idle});
    return t;
}

constexpr Table kTable = make_table();

}  // namespace

bool can_transition(CharacterState from, CharacterState to) {
    if (from == S::Count || to == S::Count) {
        return false;
    }
    return from == to ? (from != S::Attacking || kTable[at(from)][at(to)]) : kTable[at(from)][at(to)];
}

std::string_view to_string(CharacterState state) {
    switch (state) {
        case S::Idle: return "idle";
        case S::Moving: return "moving";
        case S::Running: return "running";
        case S::Attacking: return "attacking";
        case S::Casting: return "casting";
        case S::Dodging: return "dodging";
        case S::Hit: return "hit";
        case S::Resting: return "resting";
        case S::Dead: return "dead";
        case S::Transitioning: return "transitioning";
        case S::InDialogue: return "in_dialogue";
        case S::Count: break;
    }
    return "?";
}

}  // namespace wind::game
