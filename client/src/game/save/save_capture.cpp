// 이 파일은 무엇을 하나요?
// 게임 상태 → SaveData. 칸마다 그대로 옮겨 적기만 해요.

#include "game/save/save_capture.hpp"

#include "game/items/loot_store.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/world/game_clock.hpp"

#include "wind/items/equipment.hpp"
#include "wind/items/inventory.hpp"
#include "wind/items/quick_slots.hpp"

namespace wind::game {

save::SaveData capture_save(const SaveSources& sources, const HeroPlace& hero) {
    save::SaveData data;
    const HeroProgress& progress = sources.progress;
    // 주인공: 능력치·룬·HP·MP·위치
    data.stats = progress.stats();
    data.runes = progress.runes();
    data.hp = hero.hp;
    data.mp = hero.mp;
    data.position = {hero.map, hero.feet.x, hero.feet.y};
    // 부활 쿠션·찾은 쿠션·룬 흔적
    if (const auto& cushion = progress.last_cushion()) data.last_cushion = save::SavedCushion{cushion->map, cushion->id};
    data.discovered_cushions = progress.discovered_ids();
    if (const auto& trace = progress.trace()) {
        data.rune_trace = save::SavedTrace{trace->map, trace->at.x, trace->at.y, trace->amount};
    }
    // 마술 공책 (잠긴 칸에 든 마술도 그대로: 지능이 오르면 다시 열려요)
    const auto& book = progress.spells();
    data.learned_spells = book.learned();
    data.spell_slots = book.raw_slots();
    data.selected_spell_slot = static_cast<int>(book.raw_selected());
    // 가방 칸들 (빈 칸 포함, 칸 순서 그대로)
    for (std::size_t i = 0; i < sources.bag.slot_count(); ++i) data.bag.push_back(sources.bag.at(i));
    // 장비 칸 (EquipSlot 순서)
    for (std::size_t i = 0; i < data.equipment.size(); ++i) {
        data.equipment[i] = sources.worn.worn(static_cast<items::EquipSlot>(i));
    }
    for (std::size_t i = 0; i < data.quick_slots.size(); ++i) data.quick_slots[i] = sources.quick.at(i);
    // 바닥 아이템: 아이템이 있는 맵만 (튀어 오름·알림 같은 연출 상태는 버려요)
    for (const auto& [map, loot] : sources.loot.maps()) {
        if (loot.items().empty()) continue;
        save::SavedMapLoot saved{map, {}};
        for (const auto& item : loot.items()) saved.items.push_back({item.stack, item.at.x, item.at.y});
        data.ground.push_back(std::move(saved));
    }
    data.clock = {sources.clock.day(), sources.clock.hour()};
    return data;
}

}  // namespace wind::game
