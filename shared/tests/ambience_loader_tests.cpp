// 이 파일은 무엇을 하나요?
// 맵 분위기(ambience) 읽기를 숙제 검사(테스트)해요: 주변광, 주인공 빛, 맵의 빛들.
#include "wind/content/ambience_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::ErrorList;
using wind::content::parse_ambience;

bool has_error_at(const ErrorList& errors, std::string_view field) {
    return std::ranges::any_of(errors.items(), [field](const auto& e) { return e.field == field; });
}

json make_ambience() {
    return json{
        {"ambient_light", "#1c1a30"},
        {"player_light", {{"radius", 80}, {"color", "#ffe2b8"}, {"intensity", 0.55}}},
        {"lights",
         {{{"x", 3.5}, {"y", 2.2}, {"radius", 110}, {"color", "#ffb060"}, {"intensity", 1},
           {"flicker", 0.35}, {"fx", "fx/torch_embers.json"}},
          {{"x", 24.5}, {"y", 14.5}, {"radius", 140}, {"color", "#5878ff"}, {"intensity", 0.9},
           {"pulse", {{"frequency", 0.4}, {"depth", 0.5}}}}}}};
}

TEST(AmbienceLoader, ParsesLightsAndAmbient) {
    ErrorList errors;
    wind::world::Ambience out;
    parse_ambience(make_ambience(), "cave.json", 30.0f, 20.0f, errors, out);
    ASSERT_TRUE(errors.empty()) << wind::content::to_string(errors.items().front());
    ASSERT_TRUE(out.ambient_light.has_value());
    EXPECT_EQ(*out.ambient_light, (wind::world::Rgba{0x1c, 0x1a, 0x30, 255}));
    ASSERT_TRUE(out.player_light.has_value());
    EXPECT_FLOAT_EQ(out.player_light->radius, 80.0f);
    ASSERT_EQ(out.lights.size(), 2u);
    EXPECT_FLOAT_EQ(out.lights[0].x, 3.5f);
    EXPECT_FLOAT_EQ(out.lights[0].flicker, 0.35f);
    EXPECT_EQ(out.lights[0].fx, "fx/torch_embers.json");
    EXPECT_FLOAT_EQ(out.lights[1].pulse_frequency, 0.4f);
    EXPECT_TRUE(out.lights[1].fx.empty());
}

// 일부러 틀리게: 맵 밖 위치, 너무 큰 반지름, 이상한 색, 폴더 밖 효과 파일, 너무 빠른 숨쉬기
TEST(AmbienceLoader, ReportsLightFieldPaths) {
    auto doc = make_ambience();
    doc["lights"][0]["x"] = 31;
    doc["lights"][0]["radius"] = 9999;
    doc["lights"][0]["fx"] = "../escape.json";
    doc["lights"][1]["color"] = "blue";
    doc["lights"][1]["pulse"]["frequency"] = 50;
    doc["ambient_light"] = "#12";
    ErrorList errors;
    wind::world::Ambience out;
    parse_ambience(doc, "cave.json", 30.0f, 20.0f, errors, out);
    EXPECT_TRUE(has_error_at(errors, "ambience.lights[0].x"));
    EXPECT_TRUE(has_error_at(errors, "ambience.lights[0].radius"));
    EXPECT_TRUE(has_error_at(errors, "ambience.lights[0].fx"));
    EXPECT_TRUE(has_error_at(errors, "ambience.lights[1].color"));
    EXPECT_TRUE(has_error_at(errors, "ambience.lights[1].pulse.frequency"));
    EXPECT_TRUE(has_error_at(errors, "ambience.ambient_light"));
    EXPECT_TRUE(out.lights.empty());  // 틀린 빛은 넣지 않아요
}

// 모두 빠져 있어도 괜찮아요: 조명 없는 맵
TEST(AmbienceLoader, EverythingIsOptional) {
    ErrorList errors;
    wind::world::Ambience out;
    parse_ambience(json::object(), "meadow.json", 40.0f, 30.0f, errors, out);
    EXPECT_TRUE(errors.empty());
    EXPECT_FALSE(out.ambient_light.has_value());
    EXPECT_FALSE(out.player_light.has_value());
    EXPECT_TRUE(out.lights.empty());
}

// 빛은 맵 하나에 최대 64개
TEST(AmbienceLoader, LimitsLightCount) {
    auto doc = make_ambience();
    doc["lights"] = json::array();
    for (int i = 0; i < 65; ++i) {
        doc["lights"].push_back({{"x", 1}, {"y", 1}, {"radius", 50}, {"color", "#ffffff"}, {"intensity", 1}});
    }
    ErrorList errors;
    wind::world::Ambience out;
    parse_ambience(doc, "cave.json", 30.0f, 20.0f, errors, out);
    EXPECT_TRUE(has_error_at(errors, "ambience.lights"));
}

// 바깥 맵 분위기: 밤 효과, 하루 색 곡선, 날씨 표, 밤에만 켜지는 빛
TEST(AmbienceLoader, ParsesOutdoorFields) {
    json doc{{"night_fx", {"fx/fireflies.json"}},
             {"day_cycle", "time/day_cycle.json"},
             {"weather", "weather/meadow.json"},
             {"lights", {{{"x", 1}, {"y", 1}, {"radius", 50}, {"color", "#ffffff"}, {"intensity", 1},
                          {"night_only", true}}}}};
    ErrorList errors;
    wind::world::Ambience out;
    parse_ambience(doc, "meadow.json", 40.0f, 30.0f, errors, out);
    ASSERT_TRUE(errors.empty()) << wind::content::to_string(errors.items().front());
    EXPECT_EQ(out.night_fx, (std::vector<std::string>{"fx/fireflies.json"}));
    EXPECT_EQ(out.day_cycle, "time/day_cycle.json");
    EXPECT_EQ(out.weather, "weather/meadow.json");
    ASSERT_EQ(out.lights.size(), 1u);
    EXPECT_TRUE(out.lights[0].night_only);
}

// 고정 주변광(실내)과 하루 색 곡선(바깥)은 함께 쓸 수 없어요.
TEST(AmbienceLoader, RejectsFixedLightWithDayCycle) {
    json doc{{"ambient_light", "#101020"}, {"day_cycle", "time/day_cycle.json"}};
    ErrorList errors;
    wind::world::Ambience out;
    parse_ambience(doc, "x.json", 10.0f, 10.0f, errors, out);
    EXPECT_TRUE(has_error_at(errors, "ambience.day_cycle"));
}

}  // namespace
