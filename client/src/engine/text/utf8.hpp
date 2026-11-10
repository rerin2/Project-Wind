#pragma once

// 이 파일은 무엇을 하나요?
// UTF-8 글자를 한 글자씩 끊는 도우미 함수들이에요.
// 컴퓨터는 글자를 바이트(0~255 숫자)로 저장해요. 영어 "A"는 1바이트지만, 한글 "가"는 3바이트예요.
// 그래서 줄을 바꾸거나 글자를 자를 때 아무 바이트에서나 끊으면 글자가 깨져요.
// 이 함수들은 "한 글자가 몇 바이트인지"를 알려 줘서 글자 중간에서 끊지 않게 해요.
// 콘텐츠 파일은 믿을 수 없는 입력이라, 잘못된 바이트도 1바이트짜리 글자로 보고 넘어가요(멈추지 않음).

#include <cstddef>
#include <string_view>

namespace wind::engine {

// pos 위치에서 시작하는 글자 하나의 바이트 수 (1~4). pos가 끝이면 0.
// 잘못된 UTF-8이면 1을 돌려줘요 (그 바이트 하나를 글자 하나로 봐요).
[[nodiscard]] std::size_t utf8_char_length(std::string_view text, std::size_t pos);

// 글자 수 (바이트 수가 아니라 사람이 보는 글자 수)
[[nodiscard]] std::size_t utf8_count(std::string_view text);

// 마지막 글자 하나를 뺀 길이(바이트). 빈 글이면 0.
[[nodiscard]] std::size_t utf8_drop_last(std::string_view text);

}  // namespace wind::engine
