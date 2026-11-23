// 이 파일은 무엇을 하나요?
// 무기 선반: 도감의 무기 아이템마다 무기 파일을 읽어 둬요.

#include "game/items/weapon_rack.hpp"

#include "wind/content/error_list.hpp"
#include "wind/content/weapon_loader.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/log.hpp"

namespace wind::game {

WeaponRack WeaponRack::load(const std::filesystem::path& content_root, const items::ItemCatalog& catalog) {
    WeaponRack rack;
    for (const auto& id : catalog.order()) {
        const items::ItemDef* def = catalog.find(id);
        if (def == nullptr || def->weapon.empty()) continue;  // 무기만
        auto loaded = content::load_weapon_file(content_root, def->weapon);
        if (!loaded.ok()) {
            content::log_content_errors("weapon " + def->weapon, loaded.errors);  // 그 무기만 빠져요
            continue;
        }
        rack.add(id, std::move(*loaded.value));
    }
    return rack;
}

const combat::Weapon* WeaponRack::find(std::string_view item_id) const {
    const auto it = weapons_.find(item_id);
    return it != weapons_.end() ? &it->second : nullptr;
}

}  // namespace wind::game
