#include "wind/cli.hpp"

#include <gtest/gtest.h>

#include <limits>

namespace {

using wind::cli::parse_uint;

TEST(ParseUint, AcceptsValuesInRange) {
    EXPECT_EQ(parse_uint("0", 0, 10), 0u);
    EXPECT_EQ(parse_uint("10", 0, 10), 10u);
    EXPECT_EQ(parse_uint("20", 1, 120), 20u);
}

TEST(ParseUint, RejectsOutOfRange) {
    EXPECT_FALSE(parse_uint("0", 1, 120).has_value());
    EXPECT_FALSE(parse_uint("121", 1, 120).has_value());
    EXPECT_FALSE(parse_uint("99999999999999999999999", 0,
                            std::numeric_limits<std::uint64_t>::max())
                     .has_value());
}

TEST(ParseUint, RejectsMalformedText) {
    EXPECT_FALSE(parse_uint("", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("-1", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("+1", 0, 10).has_value());
    EXPECT_FALSE(parse_uint(" 1", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("1 ", 0, 10).has_value());
    EXPECT_FALSE(parse_uint("1a", 0, 10).has_value());
}

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
