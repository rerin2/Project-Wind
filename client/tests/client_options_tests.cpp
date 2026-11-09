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

// 자료 폴더와 맵 위치를 바꿀 수 있어야 하고, 맵 위치의 '\'는 '/'로 정리돼야 해요.
TEST(ClientOptions, ParsesContentDirAndMap) {
    const auto result = parse({"--content-dir", "D:/wind/content", "--map", "maps\\other.json"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->content_dir, std::filesystem::path("D:/wind/content"));
    EXPECT_EQ(result.options->map, "maps/other.json");
}

// 맵 위치가 자료 폴더 밖("..")이나 다른 드라이브("C:")를 가리키면 거절해야 해요.
TEST(ClientOptions, RejectsMapOutsideContent) {
    EXPECT_FALSE(parse({"--map", "../secret.json"}).options.has_value());
    EXPECT_FALSE(parse({"--map", "C:/maps/a.json"}).options.has_value());
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

// 스크린샷 시험용 옵션: 몇 번째 장에서 찍을지, 어디에 저장할지
TEST(ClientOptions, ParsesScreenshotOptions) {
    const auto result =
        parse({"--screenshot-at", "60", "--screenshot-at", "66", "--screenshot-dir", "D:/shots"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->screenshot_at, (std::vector<std::uint64_t>{60, 66}));
    EXPECT_EQ(result.options->screenshot_dir, std::filesystem::path("D:/shots"));
    EXPECT_FALSE(parse({"--screenshot-at", "0"}).options.has_value());
}

// 성능 시험 옵션: 0~4000만 받아요.
TEST(ClientOptions, ParsesFxStress) {
    const auto result = parse({"--fx-stress", "2000"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->fx_stress, 2000u);
    EXPECT_FALSE(parse({"--fx-stress", "5000"}).options.has_value());
}

// 빛 성능 시험 옵션: 0~64만 받아요.
TEST(ClientOptions, ParsesLightStress) {
    const auto result = parse({"--light-stress", "32"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->light_stress, 32u);
    EXPECT_FALSE(parse({"--light-stress", "65"}).options.has_value());
}

// 시험용 자동 이동: 네 방향 이름만 받아요.
TEST(ClientOptions, ParsesAutoMove) {
    const auto result = parse({"--auto-move", "left"});
    ASSERT_TRUE(result.options.has_value()) << result.error;
    EXPECT_EQ(result.options->auto_move, wind::world::Direction::Left);
    EXPECT_FALSE(parse({"--auto-move", "sideways"}).options.has_value());
}

}  // namespace
