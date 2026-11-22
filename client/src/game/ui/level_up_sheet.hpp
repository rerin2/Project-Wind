#pragma once

// 이 파일은 무엇을 하나요?
// 레벨업 화면의 "계산 종이"예요. (명세 8.3, 13c) 그리지 않고 숫자만 다뤄서 창 없이 테스트할 수 있어요.
// - 줄 6개: 능력치 다섯 개 + 맨 아래 "확정"
// - 능력치 줄에서 [−][+]: 올릴 목표를 1씩 바꿔요. 지금 능력치보다 낮게, 60보다 높게는 안 돼요.
// - 비용: 지금 레벨 -> 목표 레벨까지 한 칸씩 더한 룬 (HeroProgress::cost_to)
// - 확정: 룬을 한 번에 내고 능력치를 올려요. 모자라면 아무것도 바뀌지 않아요 (전부 성공 또는 전부 실패).
// 실제 규칙(비용·확정)은 HeroProgress가 하고, 이 종이는 "무엇을 고르고 있나"만 기억해요.

#include "game/progression/hero_progress.hpp"

#include "wind/progression/stats.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace wind::game {

class LevelUpSheet {
public:
    static constexpr std::size_t kConfirmRow = progression::kStatCount;  // 5 = "확정" 줄
    static constexpr std::size_t kRowCount = kConfirmRow + 1;

    // 지금 능력치로 목표를 맞추고 첫 줄을 골라요 (화면을 열 때마다)
    void reset(const HeroProgress& progress);
    // 위(-1)·아래(+1) 줄로 (끝에서 반대쪽 끝으로 돌아가요)
    void move(int step);
    // 고른 능력치 줄의 목표를 step만큼 (지금 능력치 ~ 60 안으로). "확정" 줄이면 아무 일 없어요. 바뀌었으면 true
    bool change(int step, const HeroProgress& progress);
    // 확정: 성공하면 true (그 뒤 새 능력치로 다시 맞춰요). 바꾼 게 없거나 룬이 모자라면 false, 아무것도 안 바뀌어요.
    bool confirm(HeroProgress& progress);

    [[nodiscard]] std::size_t row() const { return row_; }
    [[nodiscard]] const progression::Stats& target() const { return target_; }
    [[nodiscard]] int target_level() const { return progression::level_of(target_); }
    // 목표까지 드는 룬 (목표가 이상하면 nullopt)
    [[nodiscard]] std::optional<std::int64_t> cost(const HeroProgress& progress) const;
    // 지금보다 올린 능력치가 하나라도 있나요?
    [[nodiscard]] bool changed(const HeroProgress& progress) const;
    // 확정할 수 있나요? (바꾼 게 있고 룬이 충분할 때)
    [[nodiscard]] bool can_confirm(const HeroProgress& progress) const;

private:
    progression::Stats target_;  // 올리고 싶은 능력치 (지금 능력치 이상)
    std::size_t row_ = 0;        // 고른 줄
};

}  // namespace wind::game
