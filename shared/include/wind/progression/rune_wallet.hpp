#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 가진 룬(레벨업 비용이자 상점 돈) 지갑이에요. (명세 8.3, 13a)
// - 0 ~ 999,999,999개까지 가질 수 있어요. 넘치는 만큼은 버려요 (정수 넘침 방지).
// - 쓰기는 "전부 아니면 전부 안 함": 모자라면 하나도 안 줄어요.
// - 쓰러지면 절반을 잃어요: 잃는 양 = floor(가진 룬 / 2). 잃은 룬은 되찾을 수 없어요 (초기 규칙).

#include <cstdint>

namespace wind::progression {

class RuneWallet {
public:
    static constexpr std::int64_t kMax = 999'999'999;  // 소지 상한 (명세 8.3)

    // start: 처음 가진 룬 (0~상한으로 잘라요)
    explicit RuneWallet(std::int64_t start = 0);

    // amount만큼 받아요. 상한을 넘으면 넘는 만큼은 버려요. 실제로 늘어난 양을 돌려줘요. 0 이하는 무시해요.
    std::int64_t add(std::int64_t amount);
    // amount만큼 써요. 모자라면 아무것도 바꾸지 않고 false예요. 0은 쓴 셈 치고 true, 음수는 false예요.
    bool try_spend(std::int64_t amount);
    // 쓰러짐: 절반(소수점 버림)을 잃어요. 잃은 양을 돌려줘요.
    std::int64_t lose_half();

    [[nodiscard]] std::int64_t amount() const { return amount_; }

private:
    std::int64_t amount_ = 0;
};

}  // namespace wind::progression
