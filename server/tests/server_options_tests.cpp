// 서버 명령줄 옵션 파싱의 기본값·정상값·오류 입력을 검사한다.
#include "core/server_options.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

namespace {

using wind::server::parse_server_options;

// 테스트에서 인자 목록을 간단히 넘기기 위한 도우미
wind::server::ParseResult parse(std::initializer_list<std::string_view> args) {
    const std::vector<std::string_view> list(args);
    return parse_server_options(list);
}

// 인자가 없으면 문서화된 기본값(20 Hz, 무제한, info)을 쓴다.
TEST(ServerOptions, DefaultsWhenNoArguments) {
    const auto result = parse({});
    ASSERT_TRUE(result.options.has_value());
    EXPECT_EQ(result.options->tick_rate_hz, 20u);
    EXPECT_FALSE(result.options->max_ticks.has_value());
    EXPECT_EQ(result.options->log_level, wind::log::Level::Info);
}

// 모든 옵션을 한 번에 지정하면 각각 반영되어야 한다.
TEST(ServerOptions, ParsesAllOptions) {
    const auto result = parse({"--tick-rate", "30", "--max-ticks", "100", "--log-level", "debug"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->tick_rate_hz, 30u);
    EXPECT_EQ(result.options->max_ticks, 100u);
    EXPECT_EQ(result.options->log_level, wind::log::Level::Debug);
}

// 틱 레이트는 1~120 범위의 정수만 허용한다.
TEST(ServerOptions, RejectsTickRateOutOfRange) {
    EXPECT_FALSE(parse({"--tick-rate", "0"}).options.has_value());
    EXPECT_FALSE(parse({"--tick-rate", "121"}).options.has_value());
    EXPECT_FALSE(parse({"--tick-rate", "fast"}).options.has_value());
}

// --max-ticks 0은 의미가 없으므로 거부한다.
TEST(ServerOptions, RejectsZeroMaxTicks) {
    EXPECT_FALSE(parse({"--max-ticks", "0"}).options.has_value());
}

// 알 수 없는 인자는 오류 메시지에 그 이름이 나와야 하고, 값이 빠진 옵션도 거부한다.
TEST(ServerOptions, RejectsUnknownArgumentAndMissingValue) {
    const auto unknown = parse({"--bogus", "1"});
    EXPECT_FALSE(unknown.options.has_value());
    EXPECT_NE(unknown.error.find("--bogus"), std::string::npos);

    const auto missing = parse({"--tick-rate"});
    EXPECT_FALSE(missing.options.has_value());
    EXPECT_FALSE(missing.error.empty());
}

// --help와 -h는 도움말 요청으로 처리한다.
TEST(ServerOptions, HelpFlag) {
    EXPECT_TRUE(parse({"--help"}).show_help);
    EXPECT_TRUE(parse({"-h"}).show_help);
}

}  // namespace
