// 이 파일은 무엇을 하나요?
// UTF-8 글자 끊기 도우미를 숙제 검사(테스트)해요.
// - 영어 1바이트, 한글 3바이트를 한 글자로 세요
// - 잘못된 바이트나 중간에 잘린 글자도 멈추지 않고 1바이트 글자로 봐요
#include "engine/text/utf8.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using wind::engine::utf8_char_length;
using wind::engine::utf8_count;
using wind::engine::utf8_drop_last;

TEST(Utf8, CountsAsciiAndHangul) {
    const std::string text = "A가b";  // 1 + 3 + 1 바이트
    EXPECT_EQ(text.size(), 5u);
    EXPECT_EQ(utf8_count(text), 3u);
    EXPECT_EQ(utf8_char_length(text, 0), 1u);
    EXPECT_EQ(utf8_char_length(text, 1), 3u);
    EXPECT_EQ(utf8_char_length(text, 5), 0u);  // 끝
}

TEST(Utf8, DropLastRemovesWholeGlyph) {
    const std::string text = "초원";
    EXPECT_EQ(utf8_drop_last(text), 3u);  // "초"만 남아요
    EXPECT_EQ(utf8_drop_last(""), 0u);
}

// 잘못된 바이트(0xFF), 중간에 잘린 한글(앞 2바이트만), 이어지는 바이트가 틀린 글자
TEST(Utf8, InvalidBytesAreSingleCharacters) {
    const std::string broken = std::string("\xFF") + "\xEA\xB0" ;
    EXPECT_EQ(utf8_char_length(broken, 0), 1u);
    EXPECT_EQ(utf8_char_length(broken, 1), 1u);  // 3바이트 글자인데 2바이트뿐
    EXPECT_EQ(utf8_count(broken), 3u);
    const std::string wrong_tail = "\xEA\x41\x41";  // 두 번째 바이트가 10xxxxxx가 아님
    EXPECT_EQ(utf8_char_length(wrong_tail, 0), 1u);
}

}  // namespace
