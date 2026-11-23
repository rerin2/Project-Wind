#pragma once

// 이 파일은 무엇을 하나요?
// 실행 옵션 값을 읽는 작은 도우미들이에요. (15b) client_options.cpp가 300줄을 넘지 않게 따로 뒀어요.

#include "wind/progression/stats.hpp"

#include <cstddef>
#include <string_view>

namespace wind::client {

// 파일 이름에 그대로 써도 안전한 id인가요? (a-z, 0-9, _ 만, 1~max_length자. --learn이 spells/<id>.json을 열어요)
[[nodiscard]] bool is_simple_id(std::string_view id, std::size_t max_length = 64);

// "intelligence=20" -> stats의 지능을 20으로 (--stat). 이름이 틀리거나 1~60 밖이면 false이고 stats는 그대로예요.
[[nodiscard]] bool parse_stat(std::string_view text, progression::Stats& stats);

}  // namespace wind::client
