#pragma once

// 이 파일은 무엇을 하나요?
// 맵에 놓는 물건(엔티티) 정보예요. (명세 7.1 "엔티티 배치")
// 훈련용 허수아비(9c), 몬스터(11a), 캣닢 쿠션(세이브 포인트, 13b)이 있어요.
// 순수한 값만 담아서 그림·SDL과 상관없어요.

#include "wind/world/direction.hpp"

#include <cstdint>
#include <string>

namespace wind::world {

// 물건 종류 (맵 파일의 "type" 글자와 짝)
enum class EntityType : std::uint8_t {
    TrainingDummy,  // "training_dummy": HP가 있는 연습용 허수아비
    Monster,        // "monster": 돌아다니는 몬스터. 놓인 칸이 "제자리"예요 (11a)
    SavePoint,      // "save_point": 캣닢 쿠션 (13b). 지나갈 수 있고, 주인공이 그 위에 앉아요
};

// 맵에 놓인 물건 하나
struct MapEntity {
    EntityType type = EntityType::TrainingDummy;  // 무슨 물건인지
    std::string id;                               // 맵 안에서 겹치지 않는 이름 (예: "meadow_dummy_1")
    TileCoord tile;                               // 놓인 칸
    std::string data;                             // 물건 설명 파일 위치 (콘텐츠 폴더 기준). 쿠션은 비어 있어요
    std::string display_name;                     // 보여 줄 이름 (쿠션만, 선택. 비면 맵 이름을 써요, 13b)
};

// 이 종류의 물건이 서 있는 칸을 못 지나가게 막나요?
[[nodiscard]] constexpr bool blocks_movement(EntityType type) {
    switch (type) {
        case EntityType::TrainingDummy: return true;  // 허수아비는 단단해서 못 지나가요
        case EntityType::Monster: return false;       // 몬스터는 돌아다녀서 칸을 막지 않아요
        case EntityType::SavePoint: return false;     // 쿠션은 폭신해서 올라설 수 있어요 (명세 8.7)
    }
    return false;  // 모르는 종류는 막지 않아요 (로더가 미리 걸러요)
}

}  // namespace wind::world
