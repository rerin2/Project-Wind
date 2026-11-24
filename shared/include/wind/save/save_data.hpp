#pragma once

// 이 파일은 무엇을 하나요?
// 저장 파일 한 개에 들어가는 내용의 "모양"이에요. (명세 12.1, docs/SAVE_FORMAT.md 5절, 16a)
// 게임 객체(HeroProgress, Inventory 등)를 그대로 쓰지 않고 숫자·글자 묶음으로만 둬요.
// 그래서 저장 파일 읽기·쓰기가 게임 규칙이나 화면을 몰라도 되고, 테스트하기 쉬워요.
// 게임 상태 ⇄ 이 모양 바꾸기는 16b에서 게임 쪽이 해요.
// 빈 칸(빈 마술 칸, 빈 장비 칸 등)은 빈 글자 ""이고, 파일에는 null로 써요.

#include "wind/items/equip_slot.hpp"
#include "wind/items/item_stack.hpp"
#include "wind/items/quick_slots.hpp"
#include "wind/progression/spell_book.hpp"
#include "wind/progression/stats.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::save {

// 지금 저장 형식 버전 (바꾸면 save_migration.cpp에 변환 단계를 하나 더해요)
inline constexpr std::int64_t kSaveSchemaVersion = 1;

// 파일의 "equipment" 칸 이름. EquipSlot 순서와 같아요 (Weapon, Head, Body, Accessory)
inline constexpr std::array<std::string_view, items::kEquipSlotCount> kEquipSlotKeys{"weapon", "head", "body",
                                                                                     "accessory"};

// 맵 위의 한 자리 (주인공 위치)
struct SavedPlace {
    std::string map;  // 맵 파일 (콘텐츠 폴더 기준, 예: "maps/test_meadow.json")
    float x = 0.0f;   // 맵 위 픽셀 (주인공 발)
    float y = 0.0f;
    friend bool operator==(const SavedPlace&, const SavedPlace&) = default;
};

// 쿠션 하나 (마지막으로 쉰 쿠션 = 부활 자리)
struct SavedCushion {
    std::string map;
    std::string id;
    friend bool operator==(const SavedCushion&, const SavedCushion&) = default;
};

// 룬 흔적 (명세 8.3.1)
struct SavedTrace {
    std::string map;
    float x = 0.0f;
    float y = 0.0f;
    std::int64_t amount = 0;  // 담긴 룬 (1 이상)
    friend bool operator==(const SavedTrace&, const SavedTrace&) = default;
};

// 바닥에 떨어진 아이템 하나 (연출 상태는 저장하지 않아요)
struct SavedGroundItem {
    items::ItemStack stack;
    float x = 0.0f;
    float y = 0.0f;
    friend bool operator==(const SavedGroundItem&, const SavedGroundItem&) = default;
};

// 맵 하나의 바닥 아이템
struct SavedMapLoot {
    std::string map;
    std::vector<SavedGroundItem> items;
    friend bool operator==(const SavedMapLoot&, const SavedMapLoot&) = default;
};

// 게임 시계
struct SavedClock {
    int day = 0;        // 며칠째 (0부터)
    float hour = 0.0f;  // 0 이상 24 미만
    friend bool operator==(const SavedClock&, const SavedClock&) = default;
};

// 저장 파일 하나의 전체 내용 (schema_version은 읽기·쓰기가 붙여요)
struct SaveData {
    std::string game_version;          // 저장한 게임 버전 (예: "0.1.0")
    std::int64_t saved_at = 0;         // 저장한 때 (유닉스 초, UTC). 본/자동 중 새것 고르기에 써요
    double play_time_seconds = 0.0;    // 플레이 시간

    // 주인공
    progression::Stats stats;                       // 원본 능력치 다섯 개
    std::int64_t runes = 0;                         // 지갑의 룬
    int hp = 0;                                     // 지금 HP
    float mp = 0.0f;                                // 지금 MP (스태미나는 저장 안 함)
    SavedPlace position;                            // 어디에 있나
    std::optional<SavedCushion> last_cushion;       // 부활 자리 (쉰 적 없으면 없음)
    std::vector<std::string> discovered_cushions;   // 찾은 쿠션 id
    std::optional<SavedTrace> rune_trace;           // 룬 흔적 (없으면 없음)

    // 마술 공책
    std::vector<std::string> learned_spells;                              // 배운 순서
    std::array<std::string, progression::SpellBook::kMaxSlots> spell_slots;  // 칸마다 마술 id
    int selected_spell_slot = 0;                                           // 고른 칸 (0~3)

    // 아이템
    std::vector<std::optional<items::ItemStack>> bag;           // 가방 칸들 (빈 칸은 없음)
    std::array<std::string, items::kEquipSlotCount> equipment;  // EquipSlot 순서
    std::array<std::string, items::QuickSlots::kCount> quick_slots;
    std::vector<SavedMapLoot> ground;                           // 맵마다 바닥 아이템

    // 월드
    std::vector<std::string> world_flags;  // 열린 상자·처치한 보스 등 (18~19단계부터)
    SavedClock clock;

    friend bool operator==(const SaveData&, const SaveData&) = default;
};

}  // namespace wind::save
