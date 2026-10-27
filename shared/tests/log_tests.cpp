// wind::log의 수준 변환과 초기화 동작을 검사한다.
#include "wind/log.hpp"

#include <gtest/gtest.h>

namespace {

using wind::log::Level;

// 모든 수준 이름을 대소문자 구분 없이 인식해야 한다.
TEST(LogLevel, ParsesKnownNamesCaseInsensitively) {
    EXPECT_EQ(wind::log::parse_level("trace"), Level::Trace);
    EXPECT_EQ(wind::log::parse_level("DEBUG"), Level::Debug);
    EXPECT_EQ(wind::log::parse_level("Info"), Level::Info);
    EXPECT_EQ(wind::log::parse_level("warn"), Level::Warn);
    EXPECT_EQ(wind::log::parse_level("error"), Level::Error);
    EXPECT_EQ(wind::log::parse_level("critical"), Level::Critical);
    EXPECT_EQ(wind::log::parse_level("off"), Level::Off);
}

// 빈 문자열, 비슷하지만 다른 이름, 공백이 붙은 이름은 거부한다.
TEST(LogLevel, RejectsUnknownNames) {
    EXPECT_FALSE(wind::log::parse_level("").has_value());
    EXPECT_FALSE(wind::log::parse_level("warning").has_value());
    EXPECT_FALSE(wind::log::parse_level("info ").has_value());
}

// to_string 결과를 다시 parse_level에 넣으면 원래 값이 나와야 한다.
TEST(LogLevel, ToStringRoundTrips) {
    for (Level level : {Level::Trace, Level::Debug, Level::Info, Level::Warn, Level::Error,
                        Level::Critical, Level::Off}) {
        EXPECT_EQ(wind::log::parse_level(wind::log::to_string(level)), level);
    }
}

// init은 설정한 수준을 적용하고, set_level로 실행 중 바꿀 수 있어야 한다.
TEST(Log, InitAppliesConfiguredLevel) {
    wind::log::init({.component = "test", .level = Level::Warn});
    EXPECT_EQ(wind::log::level(), Level::Warn);

    wind::log::set_level(Level::Debug);
    EXPECT_EQ(wind::log::level(), Level::Debug);

    wind::log::info("log smoke test {}", 42);  // 실제 출력 경로도 한 번 거친다
    wind::log::shutdown();
}

}  // namespace
