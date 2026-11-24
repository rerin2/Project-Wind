// 이 파일은 무엇을 하나요?
// 깨어날 자리 구하기: 부활할 곳 -> 그 맵 파일 읽기 -> 시작 칸 -> 발 위치.

#include "game/save/wake_place.hpp"

#include "engine/map/coordinates.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/world/start_place.hpp"
#include "game/world/travel_targets.hpp"

#include "wind/content/error_list.hpp"
#include "wind/content/map_loader.hpp"
#include "wind/log.hpp"

namespace wind::game {

std::optional<HeroPlace> wake_place(const std::filesystem::path& content_root, const HeroProgress& progress,
                                    std::string_view current_map, int full_hp, float full_mp) {
    // 1) 부활할 곳: 마지막 쿠션(맵·쿠션 id), 없으면 지금 맵의 "default" (실제 부활과 같은 함수)
    const world::MapExit target = respawn_target(progress, current_map);
    // 2) 그 맵 파일을 읽어요 (콘텐츠 폴더 안으로 막는 검사는 로더가 해요)
    auto map = content::load_map_file(content_root, target.to_map);
    if (!map.ok()) {
        content::log_content_errors("wake map " + target.to_map, map.errors);
        return std::nullopt;
    }
    // 3) 시작 칸: 필드를 만들 때와 같은 순서로 찾아요 (그래서 불러오면 실제 부활 자리와 같은 곳)
    const auto& data = map.value->map;
    const auto start = find_start(data, load_cushions(data), target.to_spawn);
    if (!start) {
        log::error("Wake place '{}' not found in map '{}'", target.to_spawn, target.to_map);
        return std::nullopt;
    }
    // 4) 칸 -> 발 위치 (칸 가운데 아래쪽), HP·MP는 가득
    return HeroPlace{target.to_map, engine::tile_feet(start->tile, data.tile_size), full_hp, full_mp};
}

}  // namespace wind::game
