// 이 파일은 무엇을 하나요?
// 시각을 "wind_날짜_시각_밀리초.bmp" 모양의 파일 이름으로 바꾸는 곳이에요.

#include "app/screenshot_name.hpp"

#include "wind/log.hpp"

#include <format>
#include <stdexcept>

namespace wind::client {

std::string screenshot_file_name(LocalMillis when) {
    const auto seconds = std::chrono::floor<std::chrono::seconds>(when);
    const auto millis = (when - seconds).count();
    return std::format("wind_{:%Y%m%d_%H%M%S}_{:03}.bmp", seconds, millis);
}

LocalMillis local_now() {
    const auto now = std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
    try {
        return std::chrono::current_zone()->to_local(now);
    } catch (const std::runtime_error& e) {
        // 시간대 정보를 못 읽는 컴퓨터도 있어요. 그럴 땐 UTC 시각을 그대로 써요.
        log::warn("Cannot read local time zone ({}); screenshot names use UTC", e.what());
        return LocalMillis{now.time_since_epoch()};
    }
}

}  // namespace wind::client
