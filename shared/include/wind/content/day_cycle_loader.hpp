#pragma once

// 이 파일은 무엇을 하나요?
// 하루 색 곡선 파일(content/time/*.json)을 읽고 숙제 검사를 하는 도우미예요.
//   { "schema_version": 1,
//     "keys": [ { "hour": 0, "ambient": "#262c58", "night": 1 }, ... ] }
// 점은 2~32개, 시각은 0 이상 24 미만이고 점점 커져야 해요.

#include "wind/content/load_result.hpp"
#include "wind/world/day_cycle.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kDayCycleSchemaVersion = 1;

[[nodiscard]] LoadResult<world::DayCycle> parse_day_cycle(const nlohmann::json& document,
                                                          const std::string& file);

[[nodiscard]] LoadResult<world::DayCycle> load_day_cycle_file(const std::filesystem::path& content_root,
                                                              std::string_view path);

}  // namespace wind::content
