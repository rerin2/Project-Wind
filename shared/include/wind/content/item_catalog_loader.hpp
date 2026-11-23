#pragma once

// 이 파일은 무엇을 하나요?
// 아이템 도감 파일(content/items/catalog.json)을 칸마다 검사하며 읽어요. (명세 10.1, 14.4, 14a)
//   { "schema_version": 1,
//     "items": [
//       { "id": "hp_potion", "name": "회복 물약", "description": "HP를 50 채운다.",
//         "kind": "consumable", "max_stack": 20, "icon": 0, "rarity": "common",
//         "use": { "heal_hp": 50 } },
//       { "id": "longsword", "name": "롱소드", "description": "...",
//         "kind": "weapon", "max_stack": 1, "icon": 7, "rarity": "uncommon",
//         "weapon": "weapons/longsword.json" } ] }
// 규칙:
// - kind: "consumable" | "material" | "weapon". rarity: "common" | "uncommon" | "rare" (없으면 common)
// - consumable은 "use"가 꼭 있어야 하고(heal_hp·heal_mp 중 하나는 1 이상), 다른 종류는 없어야 해요.
// - weapon은 "weapon" 파일 위치가 꼭 있어야 하고 max_stack은 1이에요. 장비 칸은 weapon이에요.
//   무기 파일이 진짜로 읽히는지는 weapon_exists 도우미로 확인해요 (참조 검사).
// - id는 겹치면 안 돼요. 아이템은 256개까지.

#include "wind/content/load_result.hpp"
#include "wind/items/item_catalog.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kItemCatalogSchemaVersion = 1;
inline constexpr std::string_view kItemCatalogPath = "items/catalog.json";

// 무기 파일 위치를 받아 "이 무기 파일이 제대로 읽히나요?"에 답하는 도우미
using WeaponExists = std::function<bool(std::string_view path)>;

[[nodiscard]] LoadResult<items::ItemCatalog> parse_item_catalog(const nlohmann::json& document,
                                                                const std::string& file,
                                                                const WeaponExists& weapon_exists);

// 콘텐츠 폴더의 items/catalog.json을 읽어요. 무기 참조는 진짜 무기 파일을 읽어 확인해요.
[[nodiscard]] LoadResult<items::ItemCatalog> load_item_catalog(
    const std::filesystem::path& content_root);

}  // namespace wind::content
