#include "app/client_options.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

namespace {

using wind::client::parse_client_options;

wind::client::ClientParseResult parse(std::initializer_list<std::string_view> args) {
    const std::vector<std::string_view> list(args);
    return parse_client_options(list);
}

TEST(ClientOptions, DefaultsWhenNoArguments) {
    const auto result = parse({});
    ASSERT_TRUE(result.options.has_value());
    EXPECT_EQ(result.options->window_width, 1280u);
    EXPECT_EQ(result.options->window_height, 720u);
    EXPECT_FALSE(result.options->max_frames.has_value());
}

TEST(ClientOptions, ParsesAllOptions) {
    const auto result = parse(
        {"--width", "800", "--height", "600", "--max-frames", "10", "--log-level", "warn"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->window_width, 800u);
    EXPECT_EQ(result.options->window_height, 600u);
    EXPECT_EQ(result.options->max_frames, 10u);
    EXPECT_EQ(result.options->log_level, wind::log::Level::Warn);
}

TEST(ClientOptions, RejectsInvalidValues) {
    EXPECT_FALSE(parse({"--width", "100"}).options.has_value());
    EXPECT_FALSE(parse({"--height", "99999"}).options.has_value());
    EXPECT_FALSE(parse({"--max-frames", "0"}).options.has_value());
    EXPECT_FALSE(parse({"--log-level", "loud"}).options.has_value());
    EXPECT_FALSE(parse({"--unknown", "1"}).options.has_value());
    EXPECT_FALSE(parse({"--width"}).options.has_value());
}

}  // namespace
