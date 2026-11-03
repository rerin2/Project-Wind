// 이 파일은 무엇을 하나요?
// 파일 위치 문지기(content_path)가 제대로 지키는지 시험하는 시험지예요.
// 착한 위치는 통과시키고, 상자(콘텐츠 폴더) 밖으로 나가려는 위치는 꼭 막아야 해요.
#include "wind/content/content_path.hpp"

#include <gtest/gtest.h>

namespace {

using wind::content::normalize_content_path;

// 착한 위치들: '\'는 '/'로, 쓸데없는 "./"나 "//"는 정리되어야 해요.
TEST(ContentPath, NormalizesSeparatorsAndDotSegments) {
    EXPECT_EQ(normalize_content_path("maps/a.json"), "maps/a.json");
    EXPECT_EQ(normalize_content_path("maps\\a.json"), "maps/a.json");
    EXPECT_EQ(normalize_content_path("./maps//a.json"), "maps/a.json");
    EXPECT_EQ(normalize_content_path("a.bmp"), "a.bmp");
}

// 나쁜 위치들: 모두 거절되어야 해요.
// 빈 글자, "..", 절대 경로, 드라이브 글자("C:"), ':' 표시, 정리하면 아무것도 없는 위치,
// 눈에 안 보이는 제어 문자, 너무 긴 글자(300자)
TEST(ContentPath, RejectsEscapesAndAbsolutePaths) {
    EXPECT_FALSE(normalize_content_path("").has_value());
    EXPECT_FALSE(normalize_content_path("../secret.txt").has_value());
    EXPECT_FALSE(normalize_content_path("maps/../../x").has_value());
    EXPECT_FALSE(normalize_content_path("/etc/passwd").has_value());
    EXPECT_FALSE(normalize_content_path("\\Windows\\win.ini").has_value());
    EXPECT_FALSE(normalize_content_path("C:/Windows/win.ini").has_value());
    EXPECT_FALSE(normalize_content_path("maps/a.json:stream").has_value());
    EXPECT_FALSE(normalize_content_path("./").has_value());
    EXPECT_FALSE(normalize_content_path(std::string("maps/\x01.json")).has_value());
    EXPECT_FALSE(normalize_content_path(std::string(300, 'a')).has_value());
}

// 착한 위치는 콘텐츠 폴더 아래의 진짜 위치가 되고, 나쁜 위치는 거절되어야 해요.
TEST(ContentPath, ResolvesUnderRoot) {
    const std::filesystem::path root = "content";
    const auto resolved = wind::content::resolve_content_path(root, "maps\\a.json");
    ASSERT_TRUE(resolved.has_value());
    EXPECT_EQ(*resolved, root / "maps" / "a.json");
    EXPECT_FALSE(wind::content::resolve_content_path(root, "../a.json").has_value());
}

}  // namespace
