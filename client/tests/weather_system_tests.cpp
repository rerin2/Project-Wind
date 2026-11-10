// 이 파일은 무엇을 하나요?
// 날씨 담당(WeatherSystem)을 숙제 검사(테스트)해요.
// - 표가 없으면 늘 맑음(효과 없음)
// - 처음 날씨를 정할 수 있나
// - 바꿀 때 지금 날씨가 서서히 약해졌다가 새 날씨가 서서히 세지나
// - 세기만큼 색과 안개가 섞이나
#include "game/presentation/weather_system.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::WeatherSystem;

wind::world::WeatherTable make_table() {
    wind::world::WeatherTable table;
    table.transition_seconds = 2.0f;
    wind::world::WeatherState clear;
    clear.id = "clear";
    clear.weight = 1.0f;
    clear.min_seconds = clear.max_seconds = 1000.0f;
    wind::world::WeatherState rain;
    rain.id = "rain";
    rain.weight = 1.0f;
    rain.min_seconds = rain.max_seconds = 1000.0f;
    rain.tint = {100, 100, 200, 255};
    rain.fog = 0.5f;
    table.states = {clear, rain};
    return table;
}

TEST(WeatherSystem, NoTableMeansNoWeather) {
    WeatherSystem weather(nullptr, 1, "");
    EXPECT_EQ(weather.current(), nullptr);
    EXPECT_EQ(weather.tint(), (wind::world::Rgba{255, 255, 255, 255}));
    EXPECT_FLOAT_EQ(weather.fog(), 0.0f);
}

// 처음 날씨를 "rain"으로 정하면 처음부터 다 온 비(세기 1)예요.
TEST(WeatherSystem, StartsWithRequestedWeatherAtFullStrength) {
    const auto table = make_table();
    WeatherSystem weather(&table, 1, "rain");
    ASSERT_NE(weather.current(), nullptr);
    EXPECT_EQ(weather.current()->id, "rain");
    EXPECT_FLOAT_EQ(weather.intensity(), 1.0f);
    EXPECT_EQ(weather.tint(), (wind::world::Rgba{100, 100, 200, 255}));
    EXPECT_FLOAT_EQ(weather.fog(), 0.5f);
}

// F6(next): 결과가 바로 보이게 빨리 바뀌어요. 비가 0.4초에 걸쳐 약해지고(그동안은 아직 비),
// 그다음 맑음이 0.4초에 걸쳐 세져요. 바뀌는 중에는 다음 날씨(upcoming)도 알려 줘요.
TEST(WeatherSystem, NextFadesOutThenInQuickly) {
    const auto table = make_table();
    WeatherSystem weather(&table, 1, "rain");
    weather.next();
    ASSERT_NE(weather.upcoming(), nullptr);
    EXPECT_EQ(weather.upcoming()->id, "clear");
    weather.update(0.2f);
    EXPECT_EQ(weather.current()->id, "rain");
    EXPECT_NEAR(weather.intensity(), 0.5f, 1e-4f);
    EXPECT_NEAR(weather.fog(), 0.25f, 1e-4f);  // 세기만큼 안개도 옅어져요
    weather.update(0.2f);
    EXPECT_EQ(weather.current()->id, "clear");
    EXPECT_EQ(weather.upcoming(), nullptr);
    EXPECT_NEAR(weather.intensity(), 0.0f, 1e-4f);
    weather.update(0.2f);
    EXPECT_NEAR(weather.intensity(), 0.5f, 1e-4f);
    weather.update(0.2f);
    EXPECT_FLOAT_EQ(weather.intensity(), 1.0f);
}

// 저절로 바뀔 때는 날씨 표의 시간(2초)만큼 천천히 바뀌어요.
TEST(WeatherSystem, NaturalChangeUsesTableTransition) {
    auto table = make_table();
    table.states[0].weight = 0.0f;  // 다음엔 꼭 비가 오도록 맑음 확률을 0으로
    table.states[0].min_seconds = table.states[0].max_seconds = 1.0f;
    WeatherSystem weather(&table, 1, "clear");
    weather.update(1.0f);  // 맑음이 끝나고 비로 바꾸기 시작
    weather.update(1.0f);
    EXPECT_NEAR(weather.intensity(), 0.5f, 1e-4f);  // 2초 중 1초 -> 반
    EXPECT_EQ(weather.current()->id, "clear");
}

// 세기 0.5인 비의 색은 흰색과 비 색의 가운데예요.
TEST(WeatherSystem, TintBlendsWithIntensity) {
    const auto table = make_table();
    WeatherSystem weather(&table, 1, "rain");
    weather.next();
    weather.update(0.2f);  // 빠른 바꾸기 0.4초 중 0.2초 -> 세기 0.5
    const auto tint = weather.tint();
    EXPECT_EQ(tint.r, 178);  // 255와 100의 가운데 (반올림)
    EXPECT_EQ(tint.b, 228);  // 255와 200의 가운데
}

}  // namespace
