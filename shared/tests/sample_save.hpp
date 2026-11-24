#pragma once

// 이 파일은 무엇을 하나요?
// 저장 테스트들이 같이 쓰는 "모든 칸이 채워진" 저장 내용 하나예요. (16a)
// 빈 칸(null)과 찬 칸이 섞여 있어 읽기·쓰기 왕복을 한 번에 시험할 수 있어요.

#include "wind/progression/stats.hpp"
#include "wind/save/save_data.hpp"

namespace wind::save::test {

inline SaveData sample_save() {
    SaveData d;
    d.game_version = "0.1.0";
    d.saved_at = 1'795'400'000;
    d.play_time_seconds = 1234.5;
    d.stats[progression::StatId::Vigor] = 12;
    d.stats[progression::StatId::Intelligence] = 20;
    d.runes = 1500;
    d.hp = 98;
    d.mp = 42.5f;
    d.position = {"maps/test_meadow.json", 208.25f, 408.0f};
    d.last_cushion = SavedCushion{"maps/test_meadow.json", "meadow_cushion"};
    d.discovered_cushions = {"meadow_cushion", "cave_cushion"};
    d.rune_trace = SavedTrace{"maps/test_cave.json", 300.0f, 410.5f, 250};
    d.learned_spells = {"star_shard", "moon_blade"};
    d.spell_slots = {"star_shard", "", "moon_blade", ""};
    d.selected_spell_slot = 2;
    d.bag.resize(24);
    d.bag[0] = items::ItemStack{"red_potion", 3};
    d.bag[5] = items::ItemStack{"dagger", 1};
    d.equipment = {"longsword", "", "", ""};
    d.quick_slots = {"", "red_potion", "", ""};
    d.ground = {SavedMapLoot{"maps/test_meadow.json", {SavedGroundItem{{"slime_gel", 2}, 320.0f, 400.0f}}}};
    d.world_flags = {"meadow_chest_1"};
    d.clock = {2, 18.5f};
    return d;
}

}  // namespace wind::save::test
