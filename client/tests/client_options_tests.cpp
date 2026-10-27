// 클라이언트 명령줄 옵션 파싱의 기본값·정상값·오류 입력을 검사한다.
#include "app/client_options.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <vector>

namespace {

using wind::client::parse_client_options;

// 테스트에서 인자 목록을 간단히 넘기기 위한 도우미
wind::client::ClientParseResult parse(std::initializer_list<std::string_view> args) {
    const std::vector<std::string_view> list(args);
    return parse_client_options(list);
}

// 인자가 없으면 1280x720, 프레임 제한 없음이 기본값이다.
TEST(ClientOptions, DefaultsWhenNoArguments) {
    const auto result = parse({});
    ASSERT_TRUE(result.options.has_value());
    EXPECT_EQ(result.options->window_width, 1280u);
    EXPECT_EQ(result.options->window_height, 720u);
    EXPECT_FALSE(result.options->max_frames.has_value());
}

// 모든 옵션을 한 번에 지정하면 각각 반영되어야 한다.
TEST(ClientOptions, ParsesAllOptions) {
    const auto result = parse(
        {"--width", "800", "--height", "600", "--max-frames", "10", "--log-level", "warn"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->window_width, 800u);
    EXPECT_EQ(result.options->window_height, 600u);
    EXPECT_EQ(result.options->max_frames, 10u);
    EXPECT_EQ(result.options->log_level, wind::log::Level::Warn);
}

// 범위 밖 창 크기, 0프레임, 잘못된 수준, 알 수 없는 인자, 값 누락은 모두 거부한다.
TEST(ClientOptions, RejectsInvalidValues) {
    EXPECT_FALSE(parse({"--width", "100"}).options.has_value());
    EXPECT_FALSE(parse({"--height", "99999"}).options.has_value());
    EXPECT_FALSE(parse({"--max-frames", "0"}).options.has_value());
    EXPECT_FALSE(parse({"--log-level", "loud"}).options.has_value());
    EXPECT_FALSE(parse({"--unknown", "1"}).options.has_value());
    EXPECT_FALSE(parse({"--width"}).options.has_value());
}

}  // namespace
