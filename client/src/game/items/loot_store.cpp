// 이 파일은 무엇을 하나요?
// 맵 이름으로 바닥 아이템 목록을 찾아요 (없으면 새로 만들어요).

#include "game/items/loot_store.hpp"

#include <string>

namespace wind::game {

MapLoot& LootStore::on(std::string_view map) {
    const auto it = maps_.find(map);
    if (it != maps_.end()) return it->second;
    return maps_.emplace(std::string(map), MapLoot{}).first->second;  // 처음 가는 맵이면 빈 목록
}

}  // namespace wind::game
