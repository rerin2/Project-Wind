// 이 파일은 무엇을 하나요?
// 캐릭터 상태 전이 표가 명세 8.5대로인지 숙제 검사해요.
#include "game/entities/character_state.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::can_transition;
using wind::game::CharacterState;
using S = CharacterState;

// 쓰러지면 공격·이동·구르기·마술을 못 해요. 부활(맵 이동, 서기)만 돼요.
TEST(CharacterState, DeadCannotActOrMove) {
    for (const S to : {S::Attacking, S::Moving, S::Running, S::Dodging, S::Casting, S::Hit}) {
        EXPECT_FALSE(can_transition(S::Dead, to)) << wind::game::to_string(to);
    }
    EXPECT_TRUE(can_transition(S::Dead, S::Transitioning));
    EXPECT_TRUE(can_transition(S::Dead, S::Idle));
}

// 서 있거나 걸을 때는 공격·구르기·마술을 시작할 수 있어요.
TEST(CharacterState, FreeStatesCanStartActions) {
    for (const S from : {S::Idle, S::Moving, S::Running}) {
        EXPECT_TRUE(can_transition(from, S::Attacking));
        EXPECT_TRUE(can_transition(from, S::Dodging));
        EXPECT_TRUE(can_transition(from, S::Casting));
    }
}

// 공격 중: 다음 콤보(공격 -> 공격)와 구르기 길이 있어요 (조건은 AttackRunner가 확인).
// 마술로는 바로 못 넘어가요.
TEST(CharacterState, AttackingLinks) {
    EXPECT_TRUE(can_transition(S::Attacking, S::Attacking));
    EXPECT_TRUE(can_transition(S::Attacking, S::Dodging));
    EXPECT_TRUE(can_transition(S::Attacking, S::Hit));
    EXPECT_FALSE(can_transition(S::Attacking, S::Casting));
    EXPECT_FALSE(can_transition(S::Attacking, S::Resting));
}

// 구르는 중에는 공격·마술을 시작하지 못해요 (구르기가 끝나야 해요).
TEST(CharacterState, DodgingMustFinishFirst) {
    EXPECT_FALSE(can_transition(S::Dodging, S::Attacking));
    EXPECT_FALSE(can_transition(S::Dodging, S::Casting));
}

// 같은 상태 그대로 있는 것은 언제나 돼요 (상태가 바뀌지 않으니까요).
TEST(CharacterState, StayingIsAlwaysAllowed) {
    EXPECT_TRUE(can_transition(S::Dodging, S::Dodging));
    EXPECT_TRUE(can_transition(S::Dead, S::Dead));
}

// 쉬는 중·대화 중에는 게임 입력이 막혀요: 서기로만 돌아가요.
TEST(CharacterState, MenusBlockGameInput) {
    for (const S from : {S::Resting, S::InDialogue}) {
        EXPECT_FALSE(can_transition(from, S::Attacking));
        EXPECT_FALSE(can_transition(from, S::Moving));
        EXPECT_FALSE(can_transition(from, S::Dodging));
        EXPECT_TRUE(can_transition(from, S::Idle));
    }
}

// 맞아서 움찔하는 동안은 행동 불가, 또 맞거나 쓰러질 수는 있어요.
TEST(CharacterState, HitStunBlocksActions) {
    EXPECT_FALSE(can_transition(S::Hit, S::Attacking));
    EXPECT_FALSE(can_transition(S::Hit, S::Dodging));
    EXPECT_TRUE(can_transition(S::Hit, S::Hit));
    EXPECT_TRUE(can_transition(S::Hit, S::Dead));
}

TEST(CharacterState, NamesAndCountGuard) {
    EXPECT_EQ(wind::game::to_string(S::Attacking), "attacking");
    EXPECT_EQ(wind::game::to_string(S::InDialogue), "in_dialogue");
    EXPECT_FALSE(can_transition(S::Count, S::Idle));
    EXPECT_FALSE(can_transition(S::Idle, S::Count));
}

}  // namespace
