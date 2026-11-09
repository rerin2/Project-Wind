#pragma once

// 이 파일은 무엇을 하나요?
// 스크린샷 파일 이름을 만들어요. 찍은 날짜와 시각을 이름에 넣어서 겹치지 않게 해요.
// 예: 2026년 11월 9일 오후 3시 4분 5.678초 -> wind_20261109_150405_678.bmp

#include <chrono>
#include <string>

namespace wind::client {

using LocalMillis = std::chrono::local_time<std::chrono::milliseconds>;

// 내 컴퓨터 시계 기준 시각으로 파일 이름을 만들어요.
[[nodiscard]] std::string screenshot_file_name(LocalMillis when);

// 지금 시각을 내 컴퓨터 시간대로 바꿔요. 시간대를 못 찾으면 세계 표준시(UTC)를 써요.
[[nodiscard]] LocalMillis local_now();

}  // namespace wind::client
