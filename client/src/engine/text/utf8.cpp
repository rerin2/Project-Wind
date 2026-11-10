// 이 파일은 무엇을 하나요?
// UTF-8 글자 길이를 재는 규칙이에요.
// 첫 바이트를 보면 글자 길이를 알 수 있어요:
//   0xxxxxxx -> 1바이트 (영어·숫자)
//   110xxxxx -> 2바이트
//   1110xxxx -> 3바이트 (한글)
//   11110xxx -> 4바이트 (이모지 등)
// 나머지 바이트는 모두 10xxxxxx 모양이어야 해요. 아니면 잘못된 글자예요.

#include "engine/text/utf8.hpp"

namespace wind::engine {
namespace {

bool is_continuation(unsigned char byte) {
    return (byte & 0xC0u) == 0x80u;
}

}  // namespace

std::size_t utf8_char_length(std::string_view text, std::size_t pos) {
    if (pos >= text.size()) {
        return 0;
    }
    const auto first = static_cast<unsigned char>(text[pos]);
    std::size_t length = 1;
    if (first >= 0xF0u && first <= 0xF4u) {
        length = 4;
    } else if (first >= 0xE0u && first <= 0xEFu) {
        length = 3;
    } else if (first >= 0xC2u && first <= 0xDFu) {
        length = 2;
    } else {
        return 1;  // 영어·숫자, 또는 잘못된 첫 바이트
    }
    if (pos + length > text.size()) {
        return 1;  // 글자가 중간에 잘렸어요
    }
    for (std::size_t i = 1; i < length; ++i) {
        if (!is_continuation(static_cast<unsigned char>(text[pos + i]))) {
            return 1;
        }
    }
    return length;
}

std::size_t utf8_count(std::string_view text) {
    std::size_t count = 0;
    for (std::size_t pos = 0; pos < text.size(); pos += utf8_char_length(text, pos)) {
        ++count;
    }
    return count;
}

std::size_t utf8_drop_last(std::string_view text) {
    std::size_t last_start = 0;
    for (std::size_t pos = 0; pos < text.size(); pos += utf8_char_length(text, pos)) {
        last_start = pos;
    }
    return last_start;
}

}  // namespace wind::engine
