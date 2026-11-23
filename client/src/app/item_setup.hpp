#pragma once

// 이 파일은 무엇을 하나요?
// 게임을 시작할 때 아이템 쪽 준비물을 만드는 도우미예요. (14b)
// - 아이템 도감(content/items/catalog.json)을 읽어요. 틀리면 게임을 시작하지 않아요 (맵·성장 규칙과 같아요).
// - 빈 가방(24칸)과 맵마다의 바닥 아이템 보관함을 만들어요.
// - 시험용 --fill-bag: 가방 모든 칸을 그 아이템으로 채워요 (도감에 없으면 경고하고 빈 가방).
// 셋 다 필드 화면 밖에 하나씩 있어서 맵을 옮겨도 남아요.

#include "game/items/loot_store.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <filesystem>
#include <memory>
#include <string_view>

namespace wind::client {

struct ItemSetup {
    items::ItemCatalog catalog;  // 아이템 도감
    items::Inventory bag;        // 주인공 가방 (24칸)
    game::LootStore loot;        // 맵마다의 바닥 아이템
};

// 도감을 못 읽으면 이유를 로그에 남기고 nullptr. (LootStore가 참조를 나눠 주니 옮기지 않게 힙에 둬요)
[[nodiscard]] std::unique_ptr<ItemSetup> load_item_setup(const std::filesystem::path& content_root,
                                                         std::string_view fill_bag_item);

}  // namespace wind::client
