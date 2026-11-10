#pragma once

// 이 파일은 무엇을 하나요?
// UI 입력 포커스(지금 고른 버튼) 담당이에요. (명세 13.3)
// 버튼이 줄지어 있을 때, 위·아래 키로 하나씩 옮기고 Enter로 눌러요.
// 끝에서 한 번 더 가면 반대쪽 끝으로 돌아가고, 꺼진(쓸 수 없는) 버튼은 건너뛰어요.
// 그리지 않고 번호만 다뤄서 창 없이 테스트할 수 있어요.

#include <cstddef>
#include <vector>

namespace wind::engine {

class FocusList {
public:
    // count개 버튼, 처음에는 모두 켜져 있고 첫 번째(켜진 것)를 골라요.
    explicit FocusList(std::size_t count);

    // 다음(+1) / 이전(-1) 켜진 버튼으로 옮겨요. 켜진 버튼이 없으면 그대로.
    void move(int step);
    // 버튼 켜기·끄기. 지금 고른 버튼을 끄면 다음 켜진 버튼으로 옮겨요.
    void set_enabled(std::size_t index, bool enabled);

    [[nodiscard]] std::size_t current() const { return current_; }
    [[nodiscard]] bool enabled(std::size_t index) const;
    [[nodiscard]] std::size_t size() const { return enabled_.size(); }
    // 고를 수 있는 버튼이 하나라도 있나요?
    [[nodiscard]] bool any_enabled() const;

private:
    std::vector<bool> enabled_;
    std::size_t current_ = 0;
};

}  // namespace wind::engine
