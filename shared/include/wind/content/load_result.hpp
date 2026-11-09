#pragma once

// 이 파일은 무엇을 하나요?
// 자료 파일을 읽은 "결과 상자"예요.
// 성공하면 value에 결과가 들어 있고, 실패하면 value는 비고 errors에 이유가 들어 있어요.
// (template: 어떤 종류의 물건이든 담을 수 있는 상자 모양)

#include "wind/content/error_list.hpp"

#include <optional>
#include <vector>

namespace wind::content {

template <typename T>
struct LoadResult {
    std::optional<T> value;
    std::vector<ContentError> errors;

    [[nodiscard]] bool ok() const { return value.has_value(); }
};

}  // namespace wind::content
