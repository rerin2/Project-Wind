// 이 파일은 무엇을 하나요?
// 맵의 물건(entities) 목록을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/entity_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_reader.hpp"

#include <optional>
#include <set>
#include <string_view>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxIdLength = 64;  // id 글자 수 상한 (바이트)

// 종류 글자 -> 물건 종류. 모르는 글자면 "없음"
std::optional<world::EntityType> parse_type(std::string_view text) {
    if (text == "training_dummy") return world::EntityType::TrainingDummy;  // 훈련용 허수아비
    return std::nullopt;                                                     // 모르는 종류
}

// 이 칸이 어느 등장 자리 위인가요?
bool on_spawn(const world::MapData& map, world::TileCoord tile) {
    for (const auto& spawn : map.spawn_points) {  // 등장 자리를 하나씩 보며
        if (spawn.position == tile) return true;   // 같은 칸이면 겹쳐요
    }
    return false;  // 겹치는 등장 자리 없음
}

}  // namespace

void parse_entities(const nlohmann::json& list, const std::string& file, world::MapData& map,
                    ErrorList& errors) {
    if (!list.is_array()) {                              // 목록이 아니면
        errors.add(file, "entities", "expected an array");  // 틀렸다고 적고
        return;                                          // 그만 읽어요
    }
    if (list.size() > kMaxMapEntities) {  // 너무 많으면 (실수나 나쁜 파일)
        errors.add(file, "entities", "too many entities (maximum " + std::to_string(kMaxMapEntities) + ")");
        return;
    }
    std::set<std::string> ids;                             // 이미 나온 id (중복 찾기)
    std::set<std::pair<std::int32_t, std::int32_t>> used;  // 이미 물건이 놓인 칸
    for (std::size_t i = 0; i < list.size(); ++i) {
        const std::string path = "entities[" + std::to_string(i) + "]";  // 오류 주소
        JsonReader reader(list[i], file, path, errors);                   // 이 물건 칸 읽기 도우미
        if (!reader.valid()) continue;                                     // { } 묶음이 아니면 건너뛰기

        const auto type_text = reader.string("type", 32);               // 종류 글자
        const auto type = type_text ? parse_type(*type_text) : std::nullopt;  // 종류로 바꾸기
        if (type_text && !type) {                                       // 모르는 종류면
            reader.error(reader.field_path("type"), "unknown entity type: " + *type_text);
        }
        auto id = reader.string("id", kMaxIdLength);           // 맵 안 이름
        const auto x = reader.integer("x", 0, map.width - 1);   // 가로 칸 (맵 안)
        const auto y = reader.integer("y", 0, map.height - 1);  // 세로 칸 (맵 안)
        auto data = reader.content_path("data");                // 설명 파일 (콘텐츠 폴더 안)
        if (!type || !id || !x || !y || !data) continue;        // 하나라도 틀리면 이 물건은 빼요

        const world::TileCoord tile{static_cast<std::int32_t>(*x), static_cast<std::int32_t>(*y)};
        bool ok = true;  // 아래 검사를 모두 통과했나
        if (!ids.insert(*id).second) {  // id가 겹치면
            reader.error(reader.field_path("id"), "duplicate entity id: " + *id);
            ok = false;
        }
        if (!used.insert({tile.x, tile.y}).second) {  // 같은 칸에 물건이 이미 있으면
            reader.error(path, "another entity is already on this tile");
            ok = false;
        }
        if (on_spawn(map, tile)) {  // 등장 자리 위면 (캐릭터가 물건에 갇혀요)
            reader.error(path, "entity is on a spawn point");
            ok = false;
        }
        if (map.exit_at(tile) != nullptr) {  // 출구 위면 (출구를 막아요)
            reader.error(path, "entity is inside an exit area");
            ok = false;
        }
        if (ok) {  // 다 통과한 물건만 맵에 넣어요
            map.entities.push_back({*type, std::move(*id), tile, std::move(*data)});
        }
    }
}

}  // namespace wind::content
