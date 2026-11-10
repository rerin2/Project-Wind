#pragma once

// 이 파일은 무엇을 하나요?
// 맵의 날씨 표예요. (명세 6.5)
// "이 맵에서는 맑음 3, 비 2, 눈 1의 확률로, 각각 몇 초쯤 이어진다"를 적어요.
// 날씨마다 하늘에서 내릴 효과(빗줄기, 눈송이), 땅에 생길 효과(물 튀김),
// 세상을 물들일 색(비 오면 살짝 어둡고 푸르게), 안개 짙기를 적어요.

#include "wind/world/rgba.hpp"

#include <string>
#include <vector>

namespace wind::world {

struct WeatherState {
    std::string id;               // "clear", "rain", "snow", "fog" ...
    float weight = 1.0f;          // 고를 확률 (다른 날씨와 비교한 크기)
    float min_seconds = 60.0f;    // 한 번 오면 최소 몇 초 이어지는지
    float max_seconds = 120.0f;   // 최대 몇 초
    std::vector<std::string> fx;         // 하늘 효과 파일들 (공중에 그려요)
    std::vector<std::string> ground_fx;  // 땅 효과 파일들 (발 위치 순서로 그려요)
    Rgba tint{255, 255, 255, 255};       // 주변광에 곱할 색 (흰색이면 변화 없음)
    float fog = 0.0f;                    // 안개 짙기 0~1
};

struct WeatherTable {
    float transition_seconds = 4.0f;  // 날씨가 바뀔 때 서서히 바뀌는 시간
    std::vector<WeatherState> states;
};

}  // namespace wind::world
