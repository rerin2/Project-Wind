#include "wind/log.hpp"

#include <gtest/gtest.h>

namespace {

using wind::log::Level;

TEST(LogLevel, ParsesKnownNamesCaseInsensitively) {
    EXPECT_EQ(wind::log::parse_level("trace"), Level::Trace);
    EXPECT_EQ(wind::log::parse_level("DEBUG"), Level::Debug);
    EXPECT_EQ(wind::log::parse_level("Info"), Level::Info);
    EXPECT_EQ(wind::log::parse_level("warn"), Level::Warn);
    EXPECT_EQ(wind::log::parse_level("error"), Level::Error);
    EXPECT_EQ(wind::log::parse_level("critical"), Level::Critical);
    EXPECT_EQ(wind::log::parse_level("off"), Level::Off);
}

TEST(LogLevel, RejectsUnknownNames) {
    EXPECT_FALSE(wind::log::parse_level("").has_value());
    EXPECT_FALSE(wind::log::parse_level("warning").has_value());
    EXPECT_FALSE(wind::log::parse_level("info ").has_value());
}

TEST(LogLevel, ToStringRoundTrips) {
    for (Level level : {Level::Trace, Level::Debug, Level::Info, Level::Warn, Level::Error,
                        Level::Critical, Level::Off}) {
        EXPECT_EQ(wind::log::parse_level(wind::log::to_string(level)), level);
    }
}

TEST(Log, InitAppliesConfiguredLevel) {
    wind::log::init({.component = "test", .level = Level::Warn});
    EXPECT_EQ(wind::log::level(), Level::Warn);

    wind::log::set_level(Level::Debug);
    EXPECT_EQ(wind::log::level(), Level::Debug);

    wind::log::info("log smoke test {}", 42);
    wind::log::shutdown();
}

}  // namespace
