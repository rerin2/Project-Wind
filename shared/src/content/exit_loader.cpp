// 이 파일은 무엇을 하나요?
// 출구 목록을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/exit_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_reader.hpp"

#include <utility>

namespace wind::content {

void parse_exits(const nlohmann::json& list, const std::string& file, world::MapData& map,
                 ErrorList& errors) {
    if (!list.is_array()) {
        errors.add(file, "exits", "expected an array");
        return;
    }
    if (list.size() > kMaxMapExits) {
        errors.add(file, "exits", "too many exits (maximum " + std::to_string(kMaxMapExits) + ")");
        return;
    }
    for (std::size_t i = 0; i < list.size(); ++i) {
        const std::string path = "exits[" + std::to_string(i) + "]";
        JsonReader exit_reader(list[i], file, path, errors);
        if (!exit_reader.valid()) {
            continue;
        }
        world::MapExit exit;
        bool ok = true;

        // 영역: 왼쪽 위 칸 (x, y)와 가로·세로 칸 수 (w, h). 맵 밖으로 나가면 안 돼요.
        if (const auto* area = exit_reader.object("area")) {
            JsonReader area_reader(*area, file, path + ".area", errors);
            const auto x = area_reader.integer("x", 0, map.width - 1);
            const auto y = area_reader.integer("y", 0, map.height - 1);
            const auto w = area_reader.integer("w", 1, map.width);
            const auto h = area_reader.integer("h", 1, map.height);
            if (x && y && w && h) {
                if (*x + *w > map.width || *y + *h > map.height) {
                    area_reader.error(path + ".area", "exit area goes outside the map");
                    ok = false;
                } else {
                    exit.area_min = {static_cast<std::int32_t>(*x), static_cast<std::int32_t>(*y)};
                    exit.area_max = {static_cast<std::int32_t>(*x + *w - 1),
                                     static_cast<std::int32_t>(*y + *h - 1)};
                }
            } else {
                ok = false;
            }
        } else {
            ok = false;
        }

        // 갈 맵: 콘텐츠 폴더 안의 파일이어야 해요.
        if (const auto to_map = exit_reader.string("to_map")) {
            if (auto normalized = normalize_content_path(*to_map)) {
                exit.to_map = std::move(*normalized);
            } else {
                exit_reader.error(exit_reader.field_path("to_map"),
                                  "path must stay inside the content folder: " + *to_map);
                ok = false;
            }
        } else {
            ok = false;
        }

        if (auto to_spawn = exit_reader.string("to_spawn", 64)) {
            exit.to_spawn = std::move(*to_spawn);
        } else {
            ok = false;
        }

        // 화면 가리는 방법은 없어도 돼요 (없으면 페이드). "fade" 또는 "iris"만 받아요.
        if (exit_reader.has("transition")) {
            const auto transition = exit_reader.string("transition", 16);
            if (transition == "fade") {
                exit.transition = world::ScreenTransition::Fade;
            } else if (transition == "iris") {
                exit.transition = world::ScreenTransition::Iris;
            } else {
                if (transition) {
                    exit_reader.error(exit_reader.field_path("transition"),
                                      "transition must be fade or iris: " + *transition);
                }
                ok = false;
            }
        }

        if (ok) {
            map.exits.push_back(std::move(exit));
        }
    }
}

}  // namespace wind::content
