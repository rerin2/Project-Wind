#pragma once

// 이 파일은 무엇을 하나요?
// 하루의 색 곡선이에요. (명세 6.4)
// "몇 시에 세상이 어떤 색인지"를 몇 개의 점(키)으로 적어 두면, 그 사이 시간은 부드럽게 이어요.
// 예: 6시 = 분홍 새벽, 12시 = 흰 낮, 18시 30분 = 주황 노을, 21시 = 남색 밤
// night(밤 정도, 0~1)도 같이 적어요. 밤 정도가 높을수록 가로등이 켜지고 반딧불이 나와요.

#include "wind/world/rgba.hpp"

#include <vector>

namespace wind::world {

// 곡선 위의 점 하나
struct DayKey {
    float hour = 0.0f;  // 0 이상 24 미만
    Rgba ambient{255, 255, 255, 255};  // 이 시각의 주변광 색
    float night = 0.0f;  // 밤 정도 0(낮) ~ 1(한밤)
};

// 시계가 얼마나 빨리 가는지는 여기가 아니라 게임 시계(GameClock)가 정해요.
// 맵을 옮겨도 시간은 이어져야 하니까요.
struct DayCycle {
    std::vector<DayKey> keys;  // 시각 순서대로 (최소 2개)
};

// 어떤 시각의 색과 밤 정도
struct DaySample {
    Rgba ambient{255, 255, 255, 255};
    float night = 0.0f;
};

// hour(0~24, 벗어나면 하루 단위로 감아요)의 색을 구해요.
// 마지막 점 뒤와 첫 점 앞은 자정을 넘어 이어진 것으로 보고 이어요 (23시 -> 1시가 부드럽게).
[[nodiscard]] DaySample sample_day_cycle(const DayCycle& cycle, float hour);

}  // namespace wind::world
