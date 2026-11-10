// 이 파일은 무엇을 하나요?
// 글 배치(줄바꿈·정렬·잘림)를 숙제 검사(테스트)해요. (8단계 완료 조건: 줄바꿈·잘림 단위 테스트)
// 진짜 글꼴 대신 "영어·숫자·띄어쓰기는 6픽셀, 그 밖(한글 등)은 12픽셀"인 가짜 자로 재요.
#include "engine/text/text_layout.hpp"

#include "engine/text/utf8.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::layout_text;
using wind::engine::TextAlign;
using wind::engine::TextLayoutOptions;

float fake_measure(std::string_view text) {
    float width = 0.0f;
    for (std::size_t pos = 0; pos < text.size();) {
        const std::size_t length = wind::engine::utf8_char_length(text, pos);
        width += length == 1 ? 6.0f : 12.0f;
        pos += length;
    }
    return width;
}

TextLayoutOptions box(float width, std::size_t max_lines = 0, TextAlign align = TextAlign::Left) {
    TextLayoutOptions options;
    options.max_width = width;
    options.max_lines = max_lines;
    options.line_height = 14.0f;
    options.align = align;
    return options;
}

TEST(TextLayout, NoWidthMeansOneLine) {
    const auto layout = layout_text("hello world", fake_measure, box(0.0f));
    ASSERT_EQ(layout.lines.size(), 1u);
    EXPECT_FLOAT_EQ(layout.lines[0].width, 66.0f);
    EXPECT_FLOAT_EQ(layout.width, 66.0f);
    EXPECT_FALSE(layout.truncated);
}

// "hello world"(66)는 60에 안 들어가서 띄어쓰기에서 넘어가요.
TEST(TextLayout, WrapsAtSpaces) {
    const auto layout = layout_text("hello world foo", fake_measure, box(60.0f));
    ASSERT_EQ(layout.lines.size(), 2u);
    EXPECT_EQ(layout.lines[0].text, "hello");
    EXPECT_EQ(layout.lines[1].text, "world foo");
    EXPECT_FLOAT_EQ(layout.lines[1].y, 14.0f);
    EXPECT_FLOAT_EQ(layout.height, 28.0f);
}

// 한글도 띄어쓰기(어절)에서 넘어가고, 글자 중간에서 끊지 않아요.
TEST(TextLayout, WrapsKoreanByWords) {
    const auto layout = layout_text("안녕하세요 반가워요", fake_measure, box(72.0f));
    ASSERT_EQ(layout.lines.size(), 2u);
    EXPECT_EQ(layout.lines[0].text, "안녕하세요");
    EXPECT_EQ(layout.lines[1].text, "반가워요");
}

// 띄어쓰기 없는 긴 말은 글자 단위로 끊어요 (한글 3바이트가 깨지지 않게).
TEST(TextLayout, BreaksLongWordsByCharacter) {
    const auto latin = layout_text("abcdefghij", fake_measure, box(30.0f));
    ASSERT_EQ(latin.lines.size(), 2u);
    EXPECT_EQ(latin.lines[0].text, "abcde");
    EXPECT_EQ(latin.lines[1].text, "fghij");
    const auto korean = layout_text("가나다라마바", fake_measure, box(36.0f));
    ASSERT_EQ(korean.lines.size(), 2u);
    EXPECT_EQ(korean.lines[0].text, "가나다");
    EXPECT_EQ(korean.lines[1].text, "라마바");
}

// 줄 수가 넘치면 마지막 줄을 "..."으로 끝내고, 그 줄도 상자 폭 안에 들어가요.
TEST(TextLayout, TruncatesWithEllipsis) {
    const auto layout = layout_text("one two three four five", fake_measure, box(48.0f, 2));
    ASSERT_EQ(layout.lines.size(), 2u);
    EXPECT_TRUE(layout.truncated);
    EXPECT_EQ(layout.lines[0].text, "one two");
    EXPECT_EQ(layout.lines[1].text, "three...");
    EXPECT_LE(layout.lines[1].width, 48.0f);
}

// 한 줄짜리 상자에 긴 한글 이름: 잘림 표시와 함께 상자 안에 들어가요.
TEST(TextLayout, SingleLineKoreanTruncates) {
    const auto layout = layout_text("아주아주긴지역이름", fake_measure, box(60.0f, 1));
    ASSERT_EQ(layout.lines.size(), 1u);
    EXPECT_TRUE(layout.truncated);
    EXPECT_EQ(layout.lines[0].text, "아주아...");  // 36 + 18 = 54
    EXPECT_LE(layout.lines[0].width, 60.0f);
}

TEST(TextLayout, FitsWithoutTruncation) {
    const auto layout = layout_text("시험 초원", fake_measure, box(100.0f, 1));
    EXPECT_FALSE(layout.truncated);
}

// 가운데 정렬은 남는 폭의 절반(정수 픽셀), 오른쪽 정렬은 남는 폭 전부만큼 밀어요.
TEST(TextLayout, AlignsLines) {
    const auto center = layout_text("abc", fake_measure, box(100.0f, 0, TextAlign::Center));
    EXPECT_FLOAT_EQ(center.lines[0].x, 41.0f);  // (100 - 18) / 2 = 41
    const auto right = layout_text("abc", fake_measure, box(100.0f, 0, TextAlign::Right));
    EXPECT_FLOAT_EQ(right.lines[0].x, 82.0f);
}

TEST(TextLayout, NewlinesStartNewLines) {
    const auto layout = layout_text("a\n\nb", fake_measure, box(100.0f));
    ASSERT_EQ(layout.lines.size(), 3u);
    EXPECT_EQ(layout.lines[1].text, "");
    EXPECT_FLOAT_EQ(layout.lines[2].y, 28.0f);
}

}  // namespace
