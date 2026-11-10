#pragma once

// 이 파일은 무엇을 하나요?
// 날씨 담당이에요. (명세 6.5)
// 맵의 날씨 표에서 확률(weight)대로 날씨를 고르고, 정해진 시간이 지나면 다음 날씨로 바꿔요.
// 바꿀 때는 갑자기 바뀌지 않게, 지금 날씨를 서서히 약하게 했다가(0으로) 새 날씨를 서서히 세게 해요.
// "세기"(intensity, 0~1)를 보고 빗줄기 양, 세상 색, 안개 짙기를 정해요.
// 보여 주기 전용이라 게임 규칙에는 영향이 없어요.

#include "wind/world/rgba.hpp"
#include "wind/world/weather_table.hpp"

#include <cstddef>
#include <cstdint>
#include <random>
#include <string_view>

namespace wind::game {

class WeatherSystem {
public:
    // table이 nullptr이면 늘 맑음이에요. table은 이 물건보다 오래 살아 있어야 해요.
    // start_id: 처음 날씨를 정하고 싶을 때 (비어 있으면 확률대로)
    WeatherSystem(const world::WeatherTable* table, std::uint32_t seed, std::string_view start_id);

    void update(float dt_seconds);
    // 다음 날씨로 바꾸기 시작해요 (표의 순서대로 하나씩, 디버그 F6).
    // 결과가 바로 보이도록, 이때는 날씨 표의 시간 대신 0.4초씩 빠르게 바뀌어요.
    void next();

    // 지금 날씨 (표가 없으면 nullptr)
    [[nodiscard]] const world::WeatherState* current() const;
    [[nodiscard]] std::size_t current_index() const { return current_; }
    // 바꾸는 중이면 다음에 올 날씨, 아니면 nullptr
    [[nodiscard]] const world::WeatherState* upcoming() const;
    // 지금 날씨의 세기 0~1 (바뀌는 중이면 0과 1 사이)
    [[nodiscard]] float intensity() const { return intensity_; }
    // 세상에 곱할 색: 흰색 -> 날씨 색으로 세기만큼
    [[nodiscard]] world::Rgba tint() const;
    // 안개 짙기 = 날씨 안개 × 세기
    [[nodiscard]] float fog() const;

private:
    void start(std::size_t index);                 // 이 날씨를 서서히 시작
    [[nodiscard]] std::size_t pick_random();       // 확률대로 하나 고르기
    [[nodiscard]] float pick_duration(const world::WeatherState& state);

    const world::WeatherTable* table_;
    std::minstd_rand rng_;
    std::size_t current_ = 0;
    std::size_t queued_ = 0;     // 지금 날씨가 다 약해지면 시작할 다음 날씨
    bool switching_ = false;     // 지금 날씨를 약하게 하는 중인가
    bool quick_ = false;         // F6으로 바꾸는 중이라 빠르게 바뀌나
    float intensity_ = 1.0f;
    float remaining_ = 0.0f;     // 지금 날씨가 몇 초 더 이어질지
};

}  // namespace wind::game
