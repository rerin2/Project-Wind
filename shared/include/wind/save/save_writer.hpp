#pragma once

// 이 파일은 무엇을 하나요?
// 저장 내용(SaveData)을 저장 파일 JSON으로 바꿔요. (docs/SAVE_FORMAT.md 5절, 16a)
// 맨 위에 지금 버전(schema_version)을 붙이고, 빈 칸은 null로 써요.
// 파일에 쓰는 일은 SaveSlots가 해요(이 함수는 글로 바꾸기만).

#include "wind/save/save_data.hpp"

#include <nlohmann/json.hpp>

namespace wind::save {

// data를 저장 파일 모양 JSON으로 바꿔요. 실패하지 않아요.
[[nodiscard]] nlohmann::json save_to_json(const SaveData& data);

}  // namespace wind::save
