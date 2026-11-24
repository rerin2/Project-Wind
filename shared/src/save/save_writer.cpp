// 이 파일은 무엇을 하나요?
// SaveData를 저장 파일 JSON으로 바꾸는 곳이에요. 칸 이름과 모양은 docs/SAVE_FORMAT.md 5절과 같아요.

#include "wind/save/save_writer.hpp"

#include <string>

namespace wind::save {
namespace {

using nlohmann::json;

// 빈 글자면 null, 아니면 그 글자 (빈 마술 칸·빈 장비 칸)
json id_or_null(const std::string& id) {
    return id.empty() ? json(nullptr) : json(id);
}

// 칸 배열(마술 칸·단축 칸)을 id 또는 null 목록으로
template <std::size_t N>
json slots_json(const std::array<std::string, N>& slots) {
    json list = json::array();
    for (const auto& id : slots) {
        list.push_back(id_or_null(id));
    }
    return list;
}

// 주인공 칸 (능력치·룬·HP·MP·위치·쿠션·흔적)
json hero_json(const SaveData& data) {
    json stats = json::object();
    for (const auto id : progression::kAllStats) {
        stats[std::string(progression::stat_key(id))] = data.stats[id];
    }
    json hero = {
        {"stats", stats},
        {"runes", data.runes},
        {"hp", data.hp},
        {"mp", data.mp},
        {"position", {{"map", data.position.map}, {"x", data.position.x}, {"y", data.position.y}}},
        {"discovered_cushions", data.discovered_cushions},
    };
    hero["last_cushion"] = data.last_cushion
                               ? json{{"map", data.last_cushion->map}, {"id", data.last_cushion->id}}
                               : json(nullptr);
    hero["rune_trace"] = data.rune_trace ? json{{"map", data.rune_trace->map},
                                                {"x", data.rune_trace->x},
                                                {"y", data.rune_trace->y},
                                                {"amount", data.rune_trace->amount}}
                                         : json(nullptr);
    return hero;
}

// 가방 칸들 (빈 칸은 null)
json bag_json(const SaveData& data) {
    json bag = json::array();
    for (const auto& slot : data.bag) {
        bag.push_back(slot ? json{{"item", slot->item_id}, {"count", slot->count}} : json(nullptr));
    }
    return bag;
}

// 장비 칸 이름 → id 또는 null
json equipment_json(const SaveData& data) {
    json worn = json::object();
    for (std::size_t i = 0; i < kEquipSlotKeys.size(); ++i) {
        worn[std::string(kEquipSlotKeys[i])] = id_or_null(data.equipment[i]);
    }
    return worn;
}

// 맵마다 바닥 아이템
json ground_json(const SaveData& data) {
    json maps = json::array();
    for (const auto& loot : data.ground) {
        json items = json::array();
        for (const auto& item : loot.items) {
            items.push_back({{"item", item.stack.item_id}, {"count", item.stack.count}, {"x", item.x}, {"y", item.y}});
        }
        maps.push_back({{"map", loot.map}, {"items", items}});
    }
    return maps;
}

}  // namespace

json save_to_json(const SaveData& data) {
    return {
        {"schema_version", kSaveSchemaVersion},
        {"game_version", data.game_version},
        {"saved_at", data.saved_at},
        {"play_time_seconds", data.play_time_seconds},
        {"hero", hero_json(data)},
        {"spells",
         {{"learned", data.learned_spells},
          {"slots", slots_json(data.spell_slots)},
          {"selected", data.selected_spell_slot}}},
        {"inventory", bag_json(data)},
        {"equipment", equipment_json(data)},
        {"quick_slots", slots_json(data.quick_slots)},
        {"ground_items", ground_json(data)},
        {"world_flags", data.world_flags},
        {"clock", {{"day", data.clock.day}, {"hour", data.clock.hour}}},
    };
}

}  // namespace wind::save
