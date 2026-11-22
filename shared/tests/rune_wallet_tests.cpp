// 이 파일은 무엇을 하나요?
// 룬 지갑 검사 (13a): 상한, 전부 아니면 안 쓰기, 절반 잃기(소수점 버림).
#include "wind/progression/rune_wallet.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

using wind::progression::RuneWallet;

TEST(RuneWallet, AddsUpToCap) {
    RuneWallet wallet(RuneWallet::kMax - 5);
    EXPECT_EQ(wallet.add(12), 5);             // 5만 들어가고 나머지는 버려요
    EXPECT_EQ(wallet.amount(), RuneWallet::kMax);
    EXPECT_EQ(wallet.add(1), 0);
    EXPECT_EQ(wallet.add(std::numeric_limits<std::int64_t>::max()), 0);  // 아주 큰 수도 넘치지 않아요
    EXPECT_EQ(wallet.amount(), RuneWallet::kMax);
}

TEST(RuneWallet, IgnoresNegativeAdds) {
    RuneWallet wallet(10);
    EXPECT_EQ(wallet.add(-3), 0);
    EXPECT_EQ(wallet.add(0), 0);
    EXPECT_EQ(wallet.amount(), 10);
}

TEST(RuneWallet, StartIsClamped) {
    EXPECT_EQ(RuneWallet(-7).amount(), 0);
    EXPECT_EQ(RuneWallet(RuneWallet::kMax + 1).amount(), RuneWallet::kMax);
}

TEST(RuneWallet, SpendIsAllOrNothing) {
    RuneWallet wallet(100);
    EXPECT_FALSE(wallet.try_spend(101));  // 모자라면
    EXPECT_EQ(wallet.amount(), 100);      // 하나도 안 줄어요
    EXPECT_FALSE(wallet.try_spend(-1));   // 이상한 값도 거절
    EXPECT_TRUE(wallet.try_spend(100));   // 딱 맞으면 돼요
    EXPECT_EQ(wallet.amount(), 0);
    EXPECT_TRUE(wallet.try_spend(0));
}

TEST(RuneWallet, LosesHalfRoundingDown) {
    RuneWallet odd(25);
    EXPECT_EQ(odd.lose_half(), 12);  // floor(25 / 2)
    EXPECT_EQ(odd.amount(), 13);
    RuneWallet one(1);
    EXPECT_EQ(one.lose_half(), 0);   // 1개면 잃지 않아요
    EXPECT_EQ(one.amount(), 1);
    RuneWallet empty;
    EXPECT_EQ(empty.lose_half(), 0);
}
