// 이 파일은 무엇을 하나요?
// SaveData → 게임 상태. 새 가방·장비·단축 칸·진행 상태·바닥 아이템을 옆에서 만들어 보고,
// 모두 성공했을 때만 진짜 자리에 넣어요.

#include "game/save/save_apply.hpp"

#include "game/items/field_loot.hpp"
#include "game/items/loot_store.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/world/game_clock.hpp"

#include "wind/items/equipment.hpp"
#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/items/quick_slots.hpp"
#include "wind/log.hpp"

#include <utility>

namespace wind::game {
namespace {

// 가방: 저장된 칸 수 그대로, 칸마다 같은 자리에
std::optional<items::Inventory> rebuild_bag(const save::SaveData& data) {
    items::Inventory bag(data.bag.size());
    for (std::size_t i = 0; i < data.bag.size(); ++i) {
        if (data.bag[i] && !bag.place_at(i, *data.bag[i])) return std::nullopt;
    }
    return bag;
}

// 단축 칸: 쓸 수 있는 아이템만 (QuickSlots 규칙 그대로)
std::optional<items::QuickSlots> rebuild_quick(const save::SaveData& data, const items::ItemCatalog& catalog) {
    items::QuickSlots quick;
    for (std::size_t i = 0; i < data.quick_slots.size(); ++i) {
        if (data.quick_slots[i].empty()) continue;
        if (quick.assign(i, data.quick_slots[i], catalog) != items::QuickSlots::Result::Assigned) return std::nullopt;
    }
    return quick;
}

// 진행 상태: 능력치·룬·흔적·쿠션, 그다음 마술 공책
std::optional<HeroProgress> rebuild_progress(const save::SaveData& data, const HeroProgress& current) {
    HeroProgress next = current;  // 성장 규칙(곡선)은 그대로 쓰고 나머지를 바꿔요
    std::optional<RuneTrace> trace;
    if (data.rune_trace) {
        trace = RuneTrace{data.rune_trace->map, {data.rune_trace->x, data.rune_trace->y}, data.rune_trace->amount};
    }
    std::optional<CushionRef> cushion;
    if (data.last_cushion) cushion = CushionRef{data.last_cushion->map, data.last_cushion->id};
    if (!next.restore(data.stats, data.runes, std::move(trace), std::move(cushion), data.discovered_cushions)) {
        return std::nullopt;
    }
    if (data.selected_spell_slot < 0 ||
        !next.spells().restore(data.learned_spells, data.spell_slots,
                               static_cast<std::size_t>(data.selected_spell_slot))) {
        return std::nullopt;
    }
    return next;
}

// 바닥 아이템: 이미 다 떨어진 상태(튀어 오름 끝)로 놓아요
LootStore rebuild_loot(const save::SaveData& data) {
    LootStore loot;
    for (const auto& saved : data.ground) {
        MapLoot& map = loot.on(saved.map);
        for (const auto& item : saved.items) {
            const engine::Vec2 at{item.x, item.y};
            map.drop(item.stack, at, at);
            map.items().back().age = FieldLoot::kSettleSeconds;  // 바로 주울 수 있어요
        }
    }
    return loot;
}

}  // namespace

bool apply_save(const save::SaveData& data, SaveSources& sources, const items::ItemCatalog& catalog) {
    // 1) 옆에서 다 만들어 봐요
    auto bag = rebuild_bag(data);
    auto quick = rebuild_quick(data, catalog);
    auto progress = rebuild_progress(data, sources.progress);
    if (!bag || !quick || !progress) {
        log::error("Save could not be applied: {}", !bag ? "bag" : !quick ? "quick slots" : "hero progress");
        return false;
    }
    items::Equipment worn;
    for (std::size_t i = 0; i < data.equipment.size(); ++i) {
        worn.wear(static_cast<items::EquipSlot>(i), data.equipment[i]);
    }
    // 2) 모두 됐으면 한 번에 바꿔요 (참조로 들고 있는 쪽이 있어서 대입으로 바꿔요)
    sources.bag = std::move(*bag);
    sources.quick = std::move(*quick);
    sources.worn = std::move(worn);
    sources.progress = std::move(*progress);
    sources.loot = rebuild_loot(data);
    sources.clock.set(data.clock.day, data.clock.hour);
    return true;
}

}  // namespace wind::game
