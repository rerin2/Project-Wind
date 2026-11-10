// 이 파일은 무엇을 하나요?
// 날씨 표 파일 검사가 맞게 동작하는지 숙제 검사(테스트)를 해요.
#include "wind/content/weather_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::parse_weather_table;

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

json make_doc() {
    return json{{"schema_version", 1},
                {"transition_seconds", 4},
                {"states",
                 {{{"id", "clear"}, {"weight", 3}, {"duration", {60, 120}}},
                  {{"id", "rain"}, {"weight", 1}, {"duration", 50}, {"fx", {"fx\\rain.json"}},
                   {"tint", "#a0aabe"}, {"fog", 0.2}}}}};
}

TEST(WeatherLoader, ParsesValidTable) {
    const auto result = parse_weather_table(make_doc(), "weather/w.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& t = *result.value;
    ASSERT_EQ(t.states.size(), 2u);
    EXPECT_FLOAT_EQ(t.states[0].min_seconds, 60.0f);
    EXPECT_FLOAT_EQ(t.states[1].min_seconds, 50.0f);  // 숫자 하나면 최소 = 최대
    EXPECT_FLOAT_EQ(t.states[1].max_seconds, 50.0f);
    EXPECT_EQ(t.states[1].fx, (std::vector<std::string>{"fx/rain.json"}));
    EXPECT_EQ(t.states[1].tint.r, 0xa0);
    EXPECT_FLOAT_EQ(t.states[1].fog, 0.2f);
}

// 이름이 겹치거나, 폴더 밖 효과 파일, 안개 1 초과는 거절해요.
TEST(WeatherLoader, RejectsBadStates) {
    auto doc = make_doc();
    doc["states"][1]["id"] = "clear";
    EXPECT_TRUE(has_error_at(parse_weather_table(doc, "w.json").errors, "states[1].id"));

    auto escape = make_doc();
    escape["states"][1]["fx"] = {"../secret.json"};
    EXPECT_TRUE(has_error_at(parse_weather_table(escape, "w.json").errors, "states[1].fx[0]"));

    auto thick = make_doc();
    thick["states"][1]["fog"] = 3;
    EXPECT_TRUE(has_error_at(parse_weather_table(thick, "w.json").errors, "states[1].fog"));
}

// 모든 날씨의 확률이 0이면 고를 수 있는 게 없어서 거절해요.
TEST(WeatherLoader, NeedsSomeWeight) {
    auto doc = make_doc();
    doc["states"][0]["weight"] = 0;
    doc["states"][1]["weight"] = 0;
    EXPECT_TRUE(has_error_at(parse_weather_table(doc, "w.json").errors, "states"));
}

// 저장소의 진짜 초원 날씨 표도 통과해야 해요.
TEST(WeatherLoader, RepositoryMeadowWeatherIsValid) {
    const auto result = wind::content::load_weather_file(WIND_TEST_CONTENT_DIR, "weather/meadow.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->states.size(), 4u);
}

}  // namespace
