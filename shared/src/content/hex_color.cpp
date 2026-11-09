// 이 파일은 무엇을 하나요?
// "#rrggbbaa" 색 글자를 한 글자씩 숫자로 바꾸는 곳이에요.

#include "wind/content/hex_color.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace wind::content {
namespace {

// 16진수 글자 하나(0~9, a~f, A~F)를 숫자 0~15로 바꿔요. 아니면 -1.
int nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

}  // namespace

std::optional<world::Rgba> parse_hex_color(std::string_view text) {
    if ((text.size() != 7 && text.size() != 9) || text.front() != '#') {
        return std::nullopt;
    }
    std::array<std::uint8_t, 4> channels{0, 0, 0, 255};
    const std::size_t count = (text.size() - 1) / 2;
    for (std::size_t i = 0; i < count; ++i) {
        const int hi = nibble(text[1 + i * 2]);
        const int lo = nibble(text[2 + i * 2]);
        if (hi < 0 || lo < 0) {
            return std::nullopt;
        }
        channels[i] = static_cast<std::uint8_t>(hi * 16 + lo);
    }
    return world::Rgba{channels[0], channels[1], channels[2], channels[3]};
}

}  // namespace wind::content
