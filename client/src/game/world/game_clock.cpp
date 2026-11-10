// 이 파일은 무엇을 하나요?
// 게임 시계 바늘을 돌리는 곳이에요. 24시가 넘으면 다음 날 0시로 넘어가요.

#include "game/world/game_clock.hpp"

#include <algorithm>
#include <cmath>
#include <format>

namespace wind::game {
namespace {

constexpr float kHoursPerDay = 24.0f;

}  // namespace

GameClock::GameClock(float start_hour, float seconds_per_hour)
    : hour_(std::clamp(start_hour, 0.0f, kHoursPerDay - 0.001f)),
      seconds_per_hour_(std::max(seconds_per_hour, 0.001f)) {}

void GameClock::advance(double dt_seconds) {
    skip_hours(static_cast<float>(std::max(dt_seconds, 0.0) / static_cast<double>(seconds_per_hour_)));
}

void GameClock::skip_hours(float hours) {
    hour_ += std::max(hours, 0.0f);
    while (hour_ >= kHoursPerDay) {
        hour_ -= kHoursPerDay;
        ++day_;
    }
}

std::string GameClock::text() const {
    const int total_minutes = static_cast<int>(std::floor(hour_ * 60.0f));
    return std::format("{:02}:{:02}", total_minutes / 60, total_minutes % 60);
}

}  // namespace wind::game
