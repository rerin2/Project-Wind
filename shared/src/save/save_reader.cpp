// 이 파일은 무엇을 하나요?
// 저장 파일 JSON을 칸별로 읽으며 검사하는 곳이에요. 상한·범위는 docs/SAVE_FORMAT.md 5절 표와 같아요.
// 읽는 순서: 맨 위 칸 → 주인공 → 마술 → 가방·장비·단축 칸 → 바닥 아이템 → 월드.
// 틀린 곳을 찾아도 바로 멈추지 않고 끝까지 읽어 오류를 모아요(오류 공책은 20개까지).

#include "wind/save/save_reader.hpp"

#include "wind/content/json_reader.hpp"
#include "wind/save/save_fields.hpp"
#include "wind/progression/rune_wallet.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <string_view>
#include <utility>

namespace wind::save {
namespace {

using content::JsonReader;
using nlohmann::json;

// 상한 (손상·조작된 파일이 메모리를 다 쓰지 못하게)
constexpr std::size_t kMaxVersionLength = 32;      // game_version 글자
constexpr std::int64_t kMaxSavedAt = 9'007'199'254'740'992;  // 2^53 (double로도 정확한 정수)
constexpr double kMaxPlayTime = 1e9;               // 플레이 시간 (초, 약 31년)
constexpr std::int64_t kMaxVital = 100'000;        // HP·MP
constexpr std::size_t kMaxDiscovered = 512;        // 찾은 쿠션
constexpr std::size_t kMaxLearned = 64;            // 배운 마술
constexpr std::size_t kMaxBagSlots = 256;          // 가방 칸
constexpr std::int64_t kMaxCount = 999;            // 한 묶음 개수 (도감 max_stack 상한과 같음)
constexpr std::size_t kMaxGroundMaps = 256;        // 바닥 아이템이 있는 맵
constexpr std::size_t kMaxGroundItems = 512;       // 맵 하나의 바닥 아이템
constexpr std::size_t kMaxWorldFlags = 4096;       // 월드 깃발
constexpr std::int64_t kMaxDay = 1'000'000;        // 게임 날짜
constexpr double kHoursPerDay = 24.0;

// 능력치 다섯 칸 (모두 1~60)
void read_stats(JsonReader& hero, SaveData& data) {
    const auto* object = hero.object("stats");
    if (object == nullptr) {
        return;
    }
    JsonReader stats(*object, hero.file(), hero.field_path("stats"), hero.errors());
    for (const auto id : progression::kAllStats) {
        if (const auto value = stats.integer(progression::stat_key(id), progression::kStatMin, progression::kStatMax)) {
            data.stats[id] = static_cast<int>(*value);
        }
    }
}

// 주인공 칸
void read_hero(JsonReader& root, SaveData& data) {
    const auto* object = root.object("hero");
    if (object == nullptr) {
        return;
    }
    JsonReader hero(*object, root.file(), "hero", root.errors());
    read_stats(hero, data);
    data.runes = hero.integer("runes", 0, progression::RuneWallet::kMax).value_or(0);
    data.hp = static_cast<int>(hero.integer("hp", 0, kMaxVital).value_or(0));
    data.mp = static_cast<float>(hero.number("mp", 0.0, static_cast<double>(kMaxVital)).value_or(0.0));
    if (const auto* place = hero.object("position")) {
        JsonReader at(*place, hero.file(), hero.field_path("position"), hero.errors());
        map_point(at, data.position.map, data.position.x, data.position.y);
    }
    data.discovered_cushions = id_list(hero, "discovered_cushions", kMaxDiscovered).value_or(std::vector<std::string>{});

    // 마지막 쿠션: null 또는 { map, id }. 있으면 찾은 쿠션 목록에도 있어야 해요(쉬면 둘 다 바뀌니까)
    if (const auto* cushion = hero.raw("last_cushion"); cushion != nullptr && !cushion->is_null()) {
        JsonReader c(*cushion, hero.file(), hero.field_path("last_cushion"), hero.errors());
        const auto map = c.content_path("map");
        const auto id = c.string("id", kMaxIdLength);
        if (map && id) {
            data.last_cushion = SavedCushion{*map, *id};
            if (std::find(data.discovered_cushions.begin(), data.discovered_cushions.end(), *id) ==
                data.discovered_cushions.end()) {
                c.error(c.field_path("id"), "last cushion is not in discovered_cushions");
            }
        }
    }
    // 룬 흔적: null 또는 { map, x, y, amount }
    if (const auto* trace = hero.raw("rune_trace"); trace != nullptr && !trace->is_null()) {
        JsonReader t(*trace, hero.file(), hero.field_path("rune_trace"), hero.errors());
        SavedTrace read;
        const bool placed = map_point(t, read.map, read.x, read.y);
        const auto amount = t.integer("amount", 1, progression::RuneWallet::kMax);
        if (placed && amount) {
            read.amount = *amount;
            data.rune_trace = std::move(read);
        }
    }
}

// 마술 칸: 배운 목록, 4칸, 고른 칸. 칸에 든 마술은 배운 것이어야 해요
void read_spells(JsonReader& root, SaveData& data) {
    const auto* object = root.object("spells");
    if (object == nullptr) {
        return;
    }
    JsonReader spells(*object, root.file(), "spells", root.errors());
    data.learned_spells = id_list(spells, "learned", kMaxLearned).value_or(std::vector<std::string>{});
    if (const auto slots = slot_ids(spells, "slots", data.spell_slots.size())) {
        std::copy(slots->begin(), slots->end(), data.spell_slots.begin());
    }
    for (std::size_t i = 0; i < data.spell_slots.size(); ++i) {
        const auto& id = data.spell_slots[i];
        if (!id.empty() && std::find(data.learned_spells.begin(), data.learned_spells.end(), id) ==
                               data.learned_spells.end()) {
            spells.error(spells.index_path("slots", i), "spell " + id + " is not learned");
        }
    }
    data.selected_spell_slot = static_cast<int>(
        spells.integer("selected", 0, static_cast<std::int64_t>(progression::SpellBook::kMaxSlots) - 1).value_or(0));
}

// 아이템 묶음 { item, count }
std::optional<items::ItemStack> stack_value(JsonReader& r) {
    const auto item = r.string("item", kMaxIdLength);
    const auto count = r.integer("count", 1, kMaxCount);
    if (!item || !count) {
        return std::nullopt;
    }
    return items::ItemStack{*item, static_cast<std::int32_t>(*count)};
}

// 가방 칸들 (1~256칸, 묶음 또는 null)
void read_bag(JsonReader& root, SaveData& data) {
    const auto* list = root.array("inventory", kMaxBagSlots);
    if (list == nullptr) {
        return;
    }
    if (list->empty()) {
        root.error("inventory", "bag must have at least one slot");
        return;
    }
    for (std::size_t i = 0; i < list->size(); ++i) {
        if ((*list)[i].is_null()) {
            data.bag.emplace_back();
            continue;
        }
        JsonReader slot((*list)[i], root.file(), root.index_path("inventory", i), root.errors());
        data.bag.push_back(stack_value(slot));
    }
}

// 장비 칸 네 개 (id 또는 null)
void read_equipment(JsonReader& root, SaveData& data) {
    const auto* object = root.object("equipment");
    if (object == nullptr) {
        return;
    }
    JsonReader worn(*object, root.file(), "equipment", root.errors());
    for (std::size_t i = 0; i < kEquipSlotKeys.size(); ++i) {
        const auto* value = worn.raw(kEquipSlotKeys[i]);
        if (value != nullptr && !value->is_null()) {
            data.equipment[i] = id_value(*value, worn, worn.field_path(kEquipSlotKeys[i])).value_or("");
        }
    }
}

// 맵마다 바닥 아이템 (맵 겹침 없음)
void read_ground(JsonReader& root, SaveData& data) {
    const auto* maps = root.array("ground_items", kMaxGroundMaps);
    if (maps == nullptr) {
        return;
    }
    std::set<std::string, std::less<>> seen;
    for (std::size_t m = 0; m < maps->size(); ++m) {
        JsonReader entry((*maps)[m], root.file(), root.index_path("ground_items", m), root.errors());
        SavedMapLoot loot;
        loot.map = entry.content_path("map").value_or("");
        if (!loot.map.empty() && !seen.insert(loot.map).second) {
            entry.error(entry.field_path("map"), "duplicate map " + loot.map);
        }
        const auto* list = entry.array("items", kMaxGroundItems);
        for (std::size_t i = 0; list != nullptr && i < list->size(); ++i) {
            JsonReader item((*list)[i], root.file(), entry.index_path("items", i), root.errors());
            SavedGroundItem ground;
            const auto stack = stack_value(item);
            const auto x = item.number("x", 0.0, kMaxCoordinate);
            const auto y = item.number("y", 0.0, kMaxCoordinate);
            if (stack && x && y) {
                loot.items.push_back({*stack, static_cast<float>(*x), static_cast<float>(*y)});
            }
        }
        data.ground.push_back(std::move(loot));
    }
}

// 게임 시계 (hour는 24 미만)
void read_clock(JsonReader& root, SaveData& data) {
    const auto* object = root.object("clock");
    if (object == nullptr) {
        return;
    }
    JsonReader clock(*object, root.file(), "clock", root.errors());
    data.clock.day = static_cast<int>(clock.integer("day", 0, kMaxDay).value_or(0));
    const auto hour = clock.number("hour", 0.0, kHoursPerDay);
    if (hour && *hour >= kHoursPerDay) {
        clock.error(clock.field_path("hour"), "hour must be below 24");
    }
    data.clock.hour = static_cast<float>(hour.value_or(0.0));
}

}  // namespace

std::optional<SaveData> save_from_json(const json& doc, const std::string& file, content::ErrorList& errors) {
    // 이 파일만의 오류 공책: 앞서 적힌 오류와 섞이지 않게 따로 세고, 끝에 넘겨줘요
    content::ErrorList found;
    JsonReader root(doc, file, "", found);
    if (!root.valid() || !root.schema_version(kSaveSchemaVersion)) {
        for (auto& e : found.take()) {
            errors.add(std::move(e.file), std::move(e.field), std::move(e.message));
        }
        return std::nullopt;
    }
    SaveData data;
    data.game_version = root.string("game_version", kMaxVersionLength).value_or("");
    data.saved_at = root.integer("saved_at", 0, kMaxSavedAt).value_or(0);
    data.play_time_seconds = root.number("play_time_seconds", 0.0, kMaxPlayTime).value_or(0.0);
    read_hero(root, data);
    read_spells(root, data);
    read_bag(root, data);
    read_equipment(root, data);
    if (const auto quick = slot_ids(root, "quick_slots", data.quick_slots.size())) {
        std::copy(quick->begin(), quick->end(), data.quick_slots.begin());
    }
    read_ground(root, data);
    data.world_flags = id_list(root, "world_flags", kMaxWorldFlags).value_or(std::vector<std::string>{});
    read_clock(root, data);

    const bool ok = found.empty();
    for (auto& e : found.take()) {
        errors.add(std::move(e.file), std::move(e.field), std::move(e.message));
    }
    return ok ? std::optional{std::move(data)} : std::nullopt;
}

}  // namespace wind::save
