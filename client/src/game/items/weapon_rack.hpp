#pragma once

// 이 파일은 무엇을 하나요?
// 무기 선반이에요. (15a) 도감의 무기 아이템마다 무기 파일(content/weapons/*.json)을 미리 읽어 둬요.
// 게임 중에 무기를 바꿔 낄 때 파일을 다시 읽지 않고 여기서 꺼내 써요.
// 도감을 읽을 때 무기 파일이 읽히는지 이미 검사했어요(14a). 그래도 여기서 실패하면 기록하고 그 무기만 빼요(맨손).
// 게임 규칙 쪽 물건이라 SDL을 몰라요.

#include "wind/combat/weapon.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace wind::items {
class ItemCatalog;
}

namespace wind::game {

class WeaponRack {
public:
    // 도감의 무기 아이템마다 무기 파일을 읽어요.
    [[nodiscard]] static WeaponRack load(const std::filesystem::path& content_root, const items::ItemCatalog& catalog);

    // 무기를 직접 올려요 (테스트용)
    void add(std::string item_id, combat::Weapon weapon) { weapons_[std::move(item_id)] = std::move(weapon); }
    // 이 아이템의 무기 (무기가 아니거나 못 읽었으면 nullptr)
    [[nodiscard]] const combat::Weapon* find(std::string_view item_id) const;
    [[nodiscard]] std::size_t size() const { return weapons_.size(); }

private:
    std::map<std::string, combat::Weapon, std::less<>> weapons_;  // 아이템 id → 무기
};

}  // namespace wind::game
