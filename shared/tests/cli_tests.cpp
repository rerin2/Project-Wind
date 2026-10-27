// 명령줄 도우미(parse_uint, to_args)의 정상·경계·오류 입력을 검사한다.
#include "wind/cli.hpp"

#include <gtest/gtest.h>

#include <limits>

namespace {

using wind::cli::parse_uint;

// 범위의 양 끝 값을 포함해 정상 값은 받아들인다.
TEST(ParseUint, AcceptsValuesInRange) {
    EXPECT_EQ(parse_uint("0", 0, 10), 0u);
    EXPECT_EQ(parse_uint("10", 0, 10), 10u);
    EXPECT_EQ(parse_uint("20", 1, 120), 20u);
}

// 범위 밖 값과 uint64를 넘는 큰 수(오버플로)는 거부한다.
TEST(ParseUint, RejectsOutOfRange) {
    EXPECT_FALSE(parse_uint("0", 1, 120).has_value());
    EXPECT_FALSE(parse_uint("121", 1, 120).has_value());
    EXPECT_FALSE(parse_uint("99999999999999999999999", 0,
                            std::numeric_limits<std::uint64_t>::max())
                     .has_value());
}

// 빈 문자열, 부호, 앞뒤 공백, 숫자 뒤 문자는 모두 거부한다.
TEST(ParseUint, RejectsMalformedText) {
    EXPECT_FALSE(parse_uint("", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("-1", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("+1", 0, 10).has_value());
    EXPECT_FALSE(parse_uint(" 1", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("1 ", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("1a", 0, 10).has_value());
}

// argv의 모든 항목이 순서대로 복사되어야 한다.
TEST(ToArgs, CopiesEveryArgument) {
    char a0[] = "prog";
    char a1[] = "--flag";
    char* argv[] = {a0, a1};
    const auto args = wind::cli::to_args(2, argv);
    ASSERT_EQ(args.size(), 2u);
    EXPECT_EQ(args[0], "prog");
    EXPECT_EQ(args[1], "--flag");
}

}  // namespace
