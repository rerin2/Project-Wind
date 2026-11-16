#pragma once

// 이 파일은 무엇을 하나요?
// 허수아비 설명 파일(content/props/*.json)을 읽고 숙제 검사를 하는 도우미예요.
//   { "schema_version": 1, "id": "training_dummy", "name": "훈련용 허수아비",
//     "max_hp": 100, "defense": 0, "recover_seconds": 1.5,
//     "hurtbox": { "offset": [-10, -30], "size": [20, 30] },
//     "sprite": { "image": "props/training_dummy.bmp", "frame": [32, 48], "origin": [16, 44] } }
// 숫자가 범위를 벗어나거나 그림 위치가 콘텐츠 폴더 밖이면 오류 목록을 돌려줘요.

#include "wind/combat/training_dummy_data.hpp"
#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kTrainingDummySchemaVersion = 1;

[[nodiscard]] LoadResult<combat::TrainingDummyData> parse_training_dummy(const nlohmann::json& document,
                                                                         const std::string& file);

[[nodiscard]] LoadResult<combat::TrainingDummyData> load_training_dummy_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::content
