#include "core/server_options.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

namespace {

using wind::server::parse_server_options;

wind::server::ParseResult parse(std::initializer_list<std::string_view> args) {
    const std::vector<std::string_view> list(args);
    return parse_server_options(list);
}

TEST(ServerOptions, DefaultsWhenNoArguments) {
    const auto result = parse({});
    ASSERT_TRUE(result.options.has_value());
    EXPECT_EQ(result.options->tick_rate_hz, 20u);
    EXPECT_FALSE(result.options->max_ticks.has_value());
    EXPECT_EQ(result.options->log_level, wind::log::Level::Info);
}

TEST(ServerOptions, ParsesAllOptions) {
    const auto result = parse({"--tick-rate", "30", "--max-ticks", "100", "--log-level", "debug"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->tick_rate_hz, 30u);
    EXPECT_EQ(result.options->max_ticks, 100u);
    EXPECT_EQ(result.options->log_level, wind::log::Level::Debug);
}

TEST(ServerOptions, RejectsTickRateOutOfRange) {
    EXPECT_FALSE(parse({"--tick-rate", "0"}).options.has_value());
    EXPECT_FALSE(parse({"--tick-rate", "121"}).options.has_value());
    EXPECT_FALSE(parse({"--tick-rate", "fast"}).options.has_value());
}

TEST(ServerOptions, RejectsZeroMaxTicks) {
    EXPECT_FALSE(parse({"--max-ticks", "0"}).options.has_value());
}

TEST(ServerOptions, RejectsUnknownArgumentAndMissingValue) {
    const auto unknown = parse({"--bogus", "1"});
    EXPECT_FALSE(unknown.options.has_value());
    EXPECT_NE(unknown.error.find("--bogus"), std::string::npos);

    const auto missing = parse({"--tick-rate"});
    EXPECT_FALSE(missing.options.has_value());
    EXPECT_FALSE(missing.error.empty());
}

TEST(ServerOptions, HelpFlag) {
    EXPECT_TRUE(parse({"--help"}).show_help);
    EXPECT_TRUE(parse({"-h"}).show_help);
}

}  // namespace
