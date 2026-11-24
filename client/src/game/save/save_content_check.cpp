// 이 파일은 무엇을 하나요?
// 저장 ↔ 콘텐츠 확인의 실제 순서예요: 아이템 → 마술 → 맵. 틀린 곳을 모두 모아 적어요.

#include "game/save/save_content_check.hpp"

#include "engine/map/coordinates.hpp"
#include "game/combat/spell_shelf.hpp"
#include "game/items/weapon_rack.hpp"

#include "wind/content/map_loader.hpp"
#include "wind/items/item_catalog.hpp"

#include <map>
#include <optional>

namespace wind::game {
namespace {

// 맵 파일을 한 번씩만 읽는 작은 보관함 (같은 맵을 여러 칸이 가리켜요)
class MapCache {
public:
    explicit MapCache(std::filesystem::path root) : root_(std::move(root)) {}
    // 읽힌 맵 (못 읽으면 nullptr)
    const world::MapData* get(const std::string& path) {
        auto [it, inserted] = maps_.try_emplace(path);
        if (inserted) {
            auto loaded = content::load_map_file(root_, path);
            if (loaded.ok()) it->second = std::move(loaded.value->map);
        }
        return it->second ? &*it->second : nullptr;
    }

private:
    std::filesystem::path root_;
    std::map<std::string, std::optional<world::MapData>> maps_;
};

// 아이템 묶음 하나: 도감에 있고 개수가 한 칸 상한 이하
void check_stack(const items::ItemStack& stack, const items::ItemCatalog& catalog, const std::string& where,
                 std::vector<std::string>& problems) {
    const auto* def = catalog.find(stack.item_id);
    if (def == nullptr) {
        problems.push_back(where + ": unknown item " + stack.item_id);
    } else if (stack.count > def->max_stack) {
        problems.push_back(where + ": " + std::to_string(stack.count) + " " + stack.item_id + " is above the stack limit");
    }
}

void check_items(const save::SaveData& data, const SaveContent& content, std::vector<std::string>& problems) {
    for (std::size_t i = 0; i < data.bag.size(); ++i) {
        if (data.bag[i]) check_stack(*data.bag[i], content.catalog, "bag slot " + std::to_string(i), problems);
    }
    // 장비: 그 칸에 끼는 장비여야 하고, 무기 칸은 무기 선반에 있어야 휘두를 수 있어요
    for (std::size_t i = 0; i < data.equipment.size(); ++i) {
        const auto& id = data.equipment[i];
        if (id.empty()) continue;
        const auto* def = content.catalog.find(id);
        const auto slot = static_cast<items::EquipSlot>(i);
        if (def == nullptr || def->equip != slot) {
            problems.push_back("equipment " + std::string(save::kEquipSlotKeys[i]) + ": " + id + " does not fit");
        } else if (slot == items::EquipSlot::Weapon && content.weapons.find(id) == nullptr) {
            problems.push_back("equipment weapon: no weapon data for " + id);
        }
    }
    // 단축 칸: 쓸 수 있는 아이템만
    for (const auto& id : data.quick_slots) {
        const auto* def = id.empty() ? nullptr : content.catalog.find(id);
        if (!id.empty() && (def == nullptr || !def->use)) problems.push_back("quick slot: " + id + " is not usable");
    }
    for (const auto& loot : data.ground) {
        for (const auto& item : loot.items) check_stack(item.stack, content.catalog, "ground in " + loot.map, problems);
    }
}

void check_spells(const save::SaveData& data, const SaveContent& content, std::vector<std::string>& problems) {
    for (const auto& id : data.learned_spells) {
        if (!content.spells.load(content.content_root, id)) problems.push_back("learned spell " + id + " cannot be loaded");
    }
    // 칸에 든 마술은 요구 지능 이상 (지능은 내려가지 않으니, 넣을 때 됐다면 지금도 돼요)
    const int intelligence = data.stats[progression::StatId::Intelligence];
    for (const auto& id : data.spell_slots) {
        const auto* spell = id.empty() ? nullptr : content.spells.find(id);
        if (spell != nullptr && spell->intelligence_required > intelligence) {
            problems.push_back("spell slot " + id + " needs intelligence " + std::to_string(spell->intelligence_required));
        }
    }
}

// 맵에 이 id의 쿠션이 있나요?
bool has_cushion(const world::MapData& map, const std::string& id) {
    for (const auto& entity : map.entities) {
        if (entity.type == world::EntityType::SavePoint && entity.id == id) return true;
    }
    return false;
}

void check_maps(const save::SaveData& data, const SaveContent& content, std::vector<std::string>& problems) {
    MapCache maps(content.content_root);
    // 주인공 위치: 맵 안의 걸을 수 있는 칸 (벽 속에서 시작하지 않게)
    if (const auto* map = maps.get(data.position.map); map == nullptr) {
        problems.push_back("position map " + data.position.map + " cannot be loaded");
    } else if (!map->is_walkable(engine::world_to_tile({data.position.x, data.position.y}, map->tile_size))) {
        problems.push_back("position is not on a walkable tile of " + data.position.map);
    }
    if (data.rune_trace && maps.get(data.rune_trace->map) == nullptr) {
        problems.push_back("rune trace map " + data.rune_trace->map + " cannot be loaded");
    }
    if (data.last_cushion) {
        const auto* map = maps.get(data.last_cushion->map);
        if (map == nullptr || !has_cushion(*map, data.last_cushion->id)) {
            problems.push_back("last cushion " + data.last_cushion->id + " is not in " + data.last_cushion->map);
        }
    }
    for (const auto& loot : data.ground) {
        if (maps.get(loot.map) == nullptr) problems.push_back("ground items map " + loot.map + " cannot be loaded");
    }
}

}  // namespace

bool check_save_content(const save::SaveData& data, const SaveContent& content, std::vector<std::string>& problems) {
    const auto before = problems.size();
    check_items(data, content, problems);
    check_spells(data, content, problems);
    check_maps(data, content, problems);
    return problems.size() == before;
}

}  // namespace wind::game
