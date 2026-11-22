// 이 파일은 무엇을 하나요?
// 레벨업 화면의 계산 종이: 줄 고르기, 목표 바꾸기, 비용, 확정.

#include "game/ui/level_up_sheet.hpp"

#include <algorithm>

namespace wind::game {

void LevelUpSheet::reset(const HeroProgress& progress) {
    target_ = progress.stats();  // 처음엔 바꾼 게 없어요
    row_ = 0;
}

void LevelUpSheet::move(int step) {
    const auto count = static_cast<int>(kRowCount);
    const int next = (static_cast<int>(row_) + step % count + count) % count;  // 끝에서 반대쪽으로 돌아가요
    row_ = static_cast<std::size_t>(next);
}

bool LevelUpSheet::change(int step, const HeroProgress& progress) {
    if (row_ >= progression::kStatCount) return false;  // "확정" 줄
    const auto id = progression::kAllStats[row_];
    const int before = target_[id];
    // 지금 능력치보다 낮게는 못 내리고(내리기 없음), 60보다 높게는 못 올려요
    target_[id] = std::clamp(before + step, progress.stats()[id], progression::kStatMax);
    return target_[id] != before;
}

bool LevelUpSheet::confirm(HeroProgress& progress) {
    if (!can_confirm(progress)) return false;    // 바꾼 게 없거나 룬이 모자라요
    if (!progress.try_raise(target_)) return false;  // 규칙은 HeroProgress가 (전부 아니면 없음)
    target_ = progress.stats();                  // 올린 뒤 새 능력치로 다시 맞춰요
    return true;
}

std::optional<std::int64_t> LevelUpSheet::cost(const HeroProgress& progress) const {
    return progress.cost_to(target_);
}

bool LevelUpSheet::changed(const HeroProgress& progress) const {
    return !(target_ == progress.stats());
}

bool LevelUpSheet::can_confirm(const HeroProgress& progress) const {
    const auto price = cost(progress);
    return changed(progress) && price && *price <= progress.runes();
}

}  // namespace wind::game
