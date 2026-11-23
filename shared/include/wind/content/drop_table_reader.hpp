#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터 파일의 드롭 표 칸을 읽는 도우미예요. (명세 9.7, 14b)
//   "drops": [ { "item": "slime_jelly", "chance": 0.5, "count": [1, 2] },
//              { "item": "hp_potion", "chance": 0.15 } ]
// - 선택 칸이에요 (없으면 아무것도 안 떨어뜨려요). 줄은 8개까지.
// - chance: 0보다 크고 1 이하. count: 숫자 하나(3) 또는 [최소, 최대], 1~99 (없으면 1).
// - 아이템 id가 도감에 있는지는 여기서 모르고, 게임이 도감과 맞춰 봐요 (도감 없이도 몬스터 파일을 읽을 수 있게).

#include "wind/content/json_reader.hpp"
#include "wind/items/drop_entry.hpp"

#include <optional>
#include <vector>

namespace wind::content {

// "drops" 칸을 읽어요. 칸이 없으면 빈 목록, 틀리면 공책에 적고 nullopt.
[[nodiscard]] std::optional<std::vector<items::DropEntry>> read_drop_table(JsonReader& reader);

}  // namespace wind::content
