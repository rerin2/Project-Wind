#pragma once

// 이 파일은 무엇을 하나요?
// 모든 맵의 바닥 아이템 목록을 맵 파일 이름별로 보관해요. (14b, 개발자와 정함: 맵마다 남음)
// 필드 화면 밖(ClientApp)에 하나만 있어서, 맵을 옮기거나 쉬거나 부활해도 바닥 아이템이 남아요.
// 16단계 저장도 이것을 파일로 쓰면 돼요.

#include "game/items/map_loot.hpp"

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace wind::game {

class LootStore {
public:
    // 이 맵의 목록 (처음이면 빈 목록을 만들어요). 돌려준 참조는 LootStore가 살아 있는 동안 그대로예요.
    [[nodiscard]] MapLoot& on(std::string_view map);
    // 모든 맵의 목록 (저장용, 16b)
    [[nodiscard]] const std::map<std::string, MapLoot, std::less<>>& maps() const { return maps_; }
    // 모두 비워요 (저장을 불러오기 전에, 필드가 없을 때만 불러요: 참조가 사라져요)
    void clear() { maps_.clear(); }

private:
    std::map<std::string, MapLoot, std::less<>> maps_;  // 맵 파일 → 바닥 아이템 (std::map이라 참조가 안 바뀌어요)
};

}  // namespace wind::game
