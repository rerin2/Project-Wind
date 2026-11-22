// 이 파일은 무엇을 하나요?
// 룬 지갑: 받기(상한까지), 쓰기(전부 아니면 안 씀), 절반 잃기.

#include "wind/progression/rune_wallet.hpp"

#include <algorithm>

namespace wind::progression {

RuneWallet::RuneWallet(std::int64_t start) : amount_(std::clamp<std::int64_t>(start, 0, kMax)) {}

std::int64_t RuneWallet::add(std::int64_t amount) {
    if (amount <= 0) return 0;
    const std::int64_t room = kMax - amount_;          // 상한까지 남은 자리
    const std::int64_t added = std::min(amount, room);  // 넘치는 만큼은 버려요 (빼기라 넘침이 없어요)
    amount_ += added;
    return added;
}

bool RuneWallet::try_spend(std::int64_t amount) {
    if (amount < 0 || amount > amount_) return false;  // 이상한 값이거나 모자라면 그대로
    amount_ -= amount;
    return true;
}

std::int64_t RuneWallet::lose_half() {
    const std::int64_t lost = amount_ / 2;  // 양수끼리 나누기라 소수점 버림과 같아요
    amount_ -= lost;
    return lost;
}

}  // namespace wind::progression
