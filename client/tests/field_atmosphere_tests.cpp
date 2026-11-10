// 이 파일은 무엇을 하나요?
// 필드의 "하늘과 공기" 담당(FieldAtmosphere)을 숙제 검사(테스트)해요.
// - 실내(하루 색 곡선 없음): 시간이 몇 시든 바깥 색을 주지 않아요
// - 바깥: 시간 색 × 날씨 색, 밤 정도를 줘요
// - 날씨 효과는 지금 날씨일 때만, 밤 효과는 밤일 때만 뿜어요
#include "game/presentation/field_atmosphere.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::FieldAtmosphere;

wind::world::DayCycle make_cycle() {
    wind::world::DayCycle cycle;
    cycle.keys = {{0.0f, {40, 40, 80, 255}, 1.0f}, {12.0f, {255, 255, 255, 255}, 0.0f}};
    return cycle;
}

wind::world::WeatherTable make_rain_table() {
    wind::world::WeatherTable table;
    wind::world::WeatherState rain;
    rain.id = "rain";
    rain.min_seconds = rain.max_seconds = 1000.0f;
    rain.fx = {"fx/rain.json"};
    rain.tint = {128, 128, 128, 255};
    table.states = {rain};
    return table;
}

const wind::engine::RectF kView{0.0f, 0.0f, 640.0f, 360.0f};

TEST(FieldAtmosphere, IndoorsHasNoOutdoorLight) {
    FieldAtmosphere atmosphere(std::nullopt, std::nullopt, {}, {}, 1, "");
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool air(10);
    wind::engine::ParticlePool ground(10);
    atmosphere.update(0.016f, 23.0f, kView, random, air, ground);
    EXPECT_FALSE(atmosphere.outdoors());
    EXPECT_FALSE(atmosphere.conditions().ambient.has_value());
    EXPECT_FLOAT_EQ(atmosphere.conditions().night, 0.0f);
    EXPECT_EQ(atmosphere.weather_name(), "none");
}

// 정오 + 회색 비: 흰색 × 회색 = 회색. 밤 0이라 반딧불은 안 나와요. 빗줄기는 나와요.
TEST(FieldAtmosphere, OutdoorsMultipliesTimeAndWeather) {
    wind::engine::ParticleEffect rain_fx;
    rain_fx.lifetime = {1.0f, 1.0f};
    rain_fx.rate = 600.0f;
    wind::engine::ParticleEffect fireflies;
    fireflies.lifetime = {1.0f, 1.0f};
    fireflies.rate = 600.0f;
    const std::map<std::string, wind::engine::ParticleEffect> weather_fx{{"fx/rain.json", rain_fx}};
    const std::vector<wind::engine::ParticleEffect> night_fx{fireflies};
    FieldAtmosphere atmosphere(make_cycle(), make_rain_table(), weather_fx, night_fx, 1, "rain");

    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool air(1000);
    wind::engine::ParticlePool ground(10);
    atmosphere.update(0.1f, 12.0f, kView, random, air, ground);
    ASSERT_TRUE(atmosphere.conditions().ambient.has_value());
    EXPECT_EQ(atmosphere.conditions().ambient->r, 128);
    EXPECT_FLOAT_EQ(atmosphere.conditions().night, 0.0f);
    EXPECT_NEAR(static_cast<float>(air.size()), 60.0f, 1.0f);  // 비만 0.1초 × 600개

    // 자정: 밤 1 -> 반딧불도 나와요 (비 + 반딧불)
    air.clear();
    atmosphere.update(0.1f, 0.0f, kView, random, air, ground);
    EXPECT_FLOAT_EQ(atmosphere.conditions().night, 1.0f);
    EXPECT_NEAR(static_cast<float>(air.size()), 120.0f, 2.0f);
}

}  // namespace
