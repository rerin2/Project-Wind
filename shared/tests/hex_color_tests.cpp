// 이 파일은 무엇을 하나요?
// 색 글자("#ff8000", "#ff800080")를 색으로 바꾸는 도우미를 숙제 검사(테스트)해요.
#include "wind/content/hex_color.hpp"

#include <gtest/gtest.h>

namespace {

using wind::content::parse_hex_color;

// #rrggbb는 진하기 255, #rrggbbaa는 적힌 진하기. 대문자도 받아요.
TEST(HexColor, ParsesSixAndEightDigits) {
    const auto c = parse_hex_color("#FF8000");
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(*c, (wind::world::Rgba{255, 128, 0, 255}));
    EXPECT_EQ(parse_hex_color("#10203040"), (wind::world::Rgba{0x10, 0x20, 0x30, 0x40}));
}

// #이 없거나, 길이가 틀리거나, 16진수가 아닌 글자는 거절해요.
TEST(HexColor, RejectsMalformedText) {
    EXPECT_FALSE(parse_hex_color("FF8000").has_value());
    EXPECT_FALSE(parse_hex_color("#FF80").has_value());
    EXPECT_FALSE(parse_hex_color("#GG8000").has_value());
    EXPECT_FALSE(parse_hex_color("").has_value());
}

}  // namespace
