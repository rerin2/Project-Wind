#pragma once

// 이 파일은 무엇을 하나요?
// 맵 파일의 "entities"(맵에 놓는 물건) 목록을 읽고 숙제 검사를 하는 도우미예요. (명세 7.1)
//   "entities": [ { "type": "training_dummy", "id": "meadow_dummy_1", "x": 10, "y": 14,
//                   "data": "props/training_dummy.json" } ]
// 검사: 아는 종류인지, id가 겹치지 않는지, 칸이 맵 안인지, 설명 파일이 콘텐츠 폴더 안인지,
//       물건끼리 같은 칸에 있지 않은지, 등장 자리·출구 영역과 겹치지 않는지.
// 설명 파일이 진짜 있는지는 필드를 만들 때(field_assets) 확인해요.

#include "wind/content/error_list.hpp"
#include "wind/world/map.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>

namespace wind::content {

// 한 맵에 놓을 수 있는 물건 최대 개수
inline constexpr std::size_t kMaxMapEntities = 256;

// list: entities 목록. map의 크기·등장 자리·출구는 먼저 채워져 있어야 해요. 읽은 물건은 map.entities에 넣어요.
void parse_entities(const nlohmann::json& list, const std::string& file, world::MapData& map,
                    ErrorList& errors);

}  // namespace wind::content
