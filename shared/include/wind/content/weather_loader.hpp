#pragma once

// 이 파일은 무엇을 하나요?
// 맵의 날씨 표 파일(content/weather/*.json)을 읽고 숙제 검사를 하는 도우미예요.
//   { "schema_version": 1, "transition_seconds": 4,
//     "states": [ { "id": "clear", "weight": 3, "duration": [60, 120] },
//                 { "id": "rain", "weight": 2, "duration": [40, 80],
//                   "fx": ["fx/rain.json"], "ground_fx": ["fx/rain_splash.json"],
//                   "tint": "#aab4c8", "fog": 0.1 } ] }
// 날씨는 1~16가지, 이름은 겹치면 안 되고, 확률(weight)이 0보다 큰 날씨가 하나는 있어야 해요.

#include "wind/content/load_result.hpp"
#include "wind/world/weather_table.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kWeatherSchemaVersion = 1;

[[nodiscard]] LoadResult<world::WeatherTable> parse_weather_table(const nlohmann::json& document,
                                                                  const std::string& file);

[[nodiscard]] LoadResult<world::WeatherTable> load_weather_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::content
