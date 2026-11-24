// 이 파일은 무엇을 하나요?
// 시작 질문(16b)의 답을 숙제 검사(테스트)해요: 백업이 있으면 Enter = 백업, Esc = 새로 시작,
// 백업이 없으면 어느 키든 새로 시작, 미래 버전은 어느 키든 끄기, 키가 없으면 답 없음.
#include "game/ui/startup_question.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::StartupQuestion;
using Answer = StartupQuestion::Answer;

TEST(StartupQuestion, CorruptWithBackup) {
    const StartupQuestion q(StartupQuestion::Kind::Corrupt, true);
    EXPECT_TRUE(q.offers_backup());
    EXPECT_FALSE(q.answer(false, false).has_value());
    EXPECT_EQ(q.answer(true, false), Answer::UseBackup);
    EXPECT_EQ(q.answer(false, true), Answer::StartFresh);
}

TEST(StartupQuestion, CorruptWithoutBackup) {
    const StartupQuestion q(StartupQuestion::Kind::Corrupt, false);
    EXPECT_FALSE(q.offers_backup());
    EXPECT_EQ(q.answer(true, false), Answer::StartFresh);
    EXPECT_EQ(q.answer(false, true), Answer::StartFresh);
}

TEST(StartupQuestion, TooNewOnlyQuits) {
    const StartupQuestion q(StartupQuestion::Kind::TooNew, true);
    EXPECT_FALSE(q.offers_backup());
    EXPECT_EQ(q.answer(true, false), Answer::Quit);
    EXPECT_EQ(q.answer(false, true), Answer::Quit);
}

}  // namespace
