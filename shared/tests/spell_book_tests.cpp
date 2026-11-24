// 이 파일은 무엇을 하나요?
// 마술 공책 규칙(배우기, 요구 지능, 칸 수, Q로 고르기)을 숙제 검사(테스트)해요. (명세 8.6, 15b)
#include "wind/progression/spell_book.hpp"

#include <gtest/gtest.h>

using wind::progression::SpellBook;
using Result = SpellBook::SlotResult;

TEST(SpellBook, LearnsEachSpellOnce) {
    SpellBook book;
    EXPECT_TRUE(book.learn("star_shard"));
    EXPECT_FALSE(book.learn("star_shard"));
    EXPECT_FALSE(book.learn(""));
    EXPECT_TRUE(book.knows("star_shard"));
    EXPECT_EQ(book.learned().size(), 1u);
}

// 완료 기준: 지능이 모자란 마술은 마술 칸에 넣을 수 없음
TEST(SpellBook, LowIntelligenceCannotSlotTheSpell) {
    SpellBook book;
    book.learn("great_star_shard");  // 요구 지능 16
    EXPECT_EQ(book.put(0, "great_star_shard", 16, 10, 2), Result::NeedsIntelligence);
    EXPECT_EQ(book.put(0, "great_star_shard", 16, 15, 2), Result::NeedsIntelligence);
    EXPECT_TRUE(book.in_slot(0, 2).empty());  // 아무것도 안 바뀌었어요
    EXPECT_EQ(book.put(0, "great_star_shard", 16, 16, 2), Result::Done);  // 같으면 돼요
    EXPECT_EQ(book.in_slot(0, 2), "great_star_shard");
}

TEST(SpellBook, OnlyLearnedSpellsInOpenSlotsOnce) {
    SpellBook book;
    EXPECT_EQ(book.put(0, "moon_blade", 14, 20, 2), Result::NotLearned);
    book.learn("moon_blade");
    EXPECT_EQ(book.put(2, "moon_blade", 14, 20, 2), Result::LockedSlot);  // 칸 2개면 2번은 잠김
    EXPECT_EQ(book.put(1, "moon_blade", 14, 20, 2), Result::Done);
    EXPECT_EQ(book.put(0, "moon_blade", 14, 20, 2), Result::AlreadySlotted);
    EXPECT_TRUE(book.in_slot(0, 2).empty());
}

TEST(SpellBook, FreeSlotIsTheFirstEmptyOpenOne) {
    SpellBook book;
    for (const char* id : {"a", "b", "c"}) book.learn(id);
    EXPECT_EQ(book.put_in_free_slot("a", 10, 10, 2), Result::Done);
    EXPECT_EQ(book.put_in_free_slot("b", 10, 10, 2), Result::Done);
    EXPECT_EQ(book.put_in_free_slot("c", 10, 10, 2), Result::LockedSlot);  // 두 칸이 다 찼어요
    EXPECT_EQ(book.in_slot(0, 2), "a");
    EXPECT_EQ(book.in_slot(1, 2), "b");
    EXPECT_EQ(book.put_in_free_slot("c", 10, 20, 3), Result::Done);  // 지능 20이면 칸 3개
}

TEST(SpellBook, QCyclesThroughFilledSlotsAndSkipsEmptyOnes) {
    SpellBook book;
    for (const char* id : {"a", "b"}) book.learn(id);
    EXPECT_TRUE(book.selected(4).empty());
    EXPECT_FALSE(book.select_next(4));  // 마술이 없으면 그대로
    book.put(0, "a", 1, 10, 4);
    EXPECT_EQ(book.selected(4), "a");
    EXPECT_FALSE(book.select_next(4));  // 하나뿐이면 그대로
    book.put(3, "b", 1, 10, 4);         // 1·2번은 비어 있어요
    EXPECT_TRUE(book.select_next(4));
    EXPECT_EQ(book.selected(4), "b");
    EXPECT_EQ(book.selected_slot(4), 3u);
    EXPECT_TRUE(book.select_next(4));   // 끝에서 처음으로
    EXPECT_EQ(book.selected(4), "a");
}

TEST(SpellBook, LockedSelectedSlotFallsBackToTheFirstSpell) {
    SpellBook book;
    for (const char* id : {"a", "b"}) book.learn(id);
    book.put(0, "a", 1, 10, 4);
    book.put(3, "b", 1, 10, 4);
    book.select_next(4);
    ASSERT_EQ(book.selected(4), "b");
    EXPECT_EQ(book.selected(2), "a");     // 칸이 2개로 보이면 3번은 잠긴 칸
    EXPECT_TRUE(book.in_slot(3, 2).empty());
    book.clear(0);
    EXPECT_TRUE(book.selected(2).empty());
}

// 쿠션에서 칸에 놓기 (15f): 다른 칸에 이미 있으면 두 칸을 맞바꿔요 (개발자와 정함)
TEST(SpellBook, PlaceSwapsWithTheOtherSlot) {
    SpellBook book;
    for (const char* id : {"a", "b", "c"}) book.learn(id);
    book.put(0, "a", 1, 10, 3);
    book.put(1, "b", 1, 10, 3);
    EXPECT_EQ(book.place(0, "b", 1, 10, 3), SpellBook::SlotResult::Done);
    EXPECT_EQ(book.in_slot(0, 3), "b");
    EXPECT_EQ(book.in_slot(1, 3), "a");
    EXPECT_EQ(book.place(2, "a", 1, 10, 3), SpellBook::SlotResult::Done);  // 빈 칸과 맞바꾸면 옮기기
    EXPECT_EQ(book.in_slot(2, 3), "a");
    EXPECT_TRUE(book.in_slot(1, 3).empty());
    EXPECT_EQ(book.place(1, "c", 1, 10, 3), SpellBook::SlotResult::Done);  // 아무 데도 없으면 그냥 넣기
    EXPECT_EQ(book.in_slot(1, 3), "c");
}

// 거절되면 아무것도 안 바뀌어요 (잠긴 칸, 배우지 않음, 지능 모자람)
TEST(SpellBook, RefusedPlaceChangesNothing) {
    SpellBook book;
    for (const char* id : {"a", "b"}) book.learn(id);
    book.put(0, "a", 1, 10, 2);
    EXPECT_EQ(book.place(2, "b", 1, 10, 2), SpellBook::SlotResult::LockedSlot);
    EXPECT_EQ(book.place(0, "z", 1, 10, 2), SpellBook::SlotResult::NotLearned);
    EXPECT_EQ(book.place(0, "b", 36, 20, 2), SpellBook::SlotResult::NeedsIntelligence);
    EXPECT_EQ(book.in_slot(0, 2), "a");
    EXPECT_TRUE(book.in_slot(1, 2).empty());
}
