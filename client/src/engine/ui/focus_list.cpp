// 이 파일은 무엇을 하나요?
// 고른 버튼 번호를 옮기는 규칙이에요.

#include "engine/ui/focus_list.hpp"

namespace wind::engine {

FocusList::FocusList(std::size_t count) : enabled_(count, true) {}

void FocusList::move(int step) {
    const std::size_t count = enabled_.size();
    if (count == 0 || step == 0 || !any_enabled()) {
        return;
    }
    // 한 칸씩 옮기면서 켜진 버튼을 찾아요 (끝에서는 반대쪽으로 돌아가요).
    std::size_t index = current_;
    for (std::size_t tries = 0; tries < count; ++tries) {
        index = step > 0 ? (index + 1) % count : (index + count - 1) % count;
        if (enabled_[index]) {
            current_ = index;
            return;
        }
    }
}

void FocusList::set_enabled(std::size_t index, bool enabled) {
    if (index >= enabled_.size()) {
        return;
    }
    enabled_[index] = enabled;
    if (!enabled && index == current_) {
        move(1);
    } else if (enabled && !enabled_[current_]) {
        current_ = index;
    }
}

bool FocusList::enabled(std::size_t index) const {
    return index < enabled_.size() && enabled_[index];
}

bool FocusList::any_enabled() const {
    for (const bool on : enabled_) {
        if (on) {
            return true;
        }
    }
    return false;
}

}  // namespace wind::engine
