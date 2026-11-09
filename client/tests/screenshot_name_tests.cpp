// 이 파일은 무엇을 하나요?
// 스크린샷 파일 이름이 날짜·시각 모양으로 맞게 만들어지는지 숙제 검사(테스트)를 해요.
#include "app/screenshot_name.hpp"

#include <gtest/gtest.h>

namespace {

using namespace std::chrono;

// 2026-11-09 15:04:05.078 -> wind_20261109_150405_078.bmp
TEST(ScreenshotName, FormatsDateTimeAndMillis) {
    const auto day = local_days{year{2026} / November / 9};
    const wind::client::LocalMillis when = day + hours{15} + minutes{4} + seconds{5} + milliseconds{78};
    EXPECT_EQ(wind::client::screenshot_file_name(when), "wind_20261109_150405_078.bmp");
}

}  // namespace
