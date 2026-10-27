#pragma once

// 클라이언트·서버·도구가 함께 쓰는 최소한의 명령줄 인자 도우미.

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace wind::cli {

// argv를 string_view 목록으로 바꾼다. 각 view는 argv가 살아 있는 동안 유효하다.
[[nodiscard]] std::vector<std::string_view> to_args(int argc, char** argv);

// 문자열 전체가 [min, max] 범위의 10진 부호 없는 정수일 때만 값을 돌려준다.
// 부호(+/-), 공백, 뒤에 붙은 문자, 범위 밖 값은 모두 거부한다(nullopt).
[[nodiscard]] std::optional<std::uint64_t> parse_uint(std::string_view text,
                                                      std::uint64_t min,
                                                      std::uint64_t max);

}  // namespace wind::cli
