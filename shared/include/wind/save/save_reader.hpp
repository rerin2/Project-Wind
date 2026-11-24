#pragma once

// 이 파일은 무엇을 하나요?
// 저장 파일 JSON을 읽어 SaveData로 바꾸고, 칸마다 모양과 범위를 검사해요. (docs/SAVE_FORMAT.md 5절, 16a)
// 저장 파일은 손상되거나 손으로 고쳐질 수 있는 외부 입력이라(명세 15장), 하나라도 틀리면 통째로 거절해요.
// 콘텐츠를 모르니 아이템·마술·맵이 실제로 있는지는 보지 않아요(게임이 불러올 때 봐요).
// 버전 변환은 먼저 migrate_save로 해 두고 불러야 해요(지금 버전만 읽어요).

#include "wind/content/error_list.hpp"
#include "wind/save/save_data.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace wind::save {

// doc(지금 버전)을 SaveData로 바꿔요. 틀린 칸이 있으면 errors에 적고 nullopt예요.
// file: 오류 메시지에 보여 줄 파일 이름 (예: "slot_1.json")
[[nodiscard]] std::optional<SaveData> save_from_json(const nlohmann::json& doc, const std::string& file,
                                                     content::ErrorList& errors);

}  // namespace wind::save
