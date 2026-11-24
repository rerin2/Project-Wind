// 이 파일은 무엇을 하나요?
// 쓰러진 순간의 저장(16c)을 숙제 검사(테스트)해요.
// - wake_place: 깨어날 자리 = 마지막 쿠션(다른 맵이어도), 없으면 지금 맵의 기본 자리, HP·MP 가득
// - 16단계 완료 기준 "쓰러져서 룬을 잃은 뒤 강제 종료해도 룬이 되살아나지 않음":
//   FieldSession::save_after_fall과 같은 순서(깨어날 자리 → 본·자동 저장)로 쓴 뒤, 게임 상태를 버리고(강제 종료)
//   새로 켜서 불러오면 잃은 뒤의 룬과 새 흔적이에요. 본·자동 어느 쪽을 불러와도 같아요.
#include "game/save/wake_place.hpp"

#include "engine/map/coordinates.hpp"
#include "game/items/loot_store.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/save/save_apply.hpp"
#include "game/save/save_capture.hpp"
#include "game/save/save_picker.hpp"
#include "game/world/game_clock.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/items/equipment.hpp"
#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/items/quick_slots.hpp"
#include "wind/save/save_slots.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace {

namespace fs = std::filesystem;
using wind::game::HeroProgress;
using wind::game::wake_place;
using wind::save::SaveKind;

constexpr const char* kMeadow = "maps/test_meadow.json";
constexpr const char* kCave = "maps/test_cave.json";
constexpr std::int32_t kTile = 32;  // test_meadow의 칸 크기

HeroProgress fresh_progress() { return *HeroProgress::load(WIND_TEST_CONTENT_DIR); }

// 필드 밖 게임 상태 한 벌 (save_apply_tests와 같은 구성)
struct World {
    HeroProgress progress = fresh_progress();
    wind::items::ItemCatalog catalog = *wind::content::load_item_catalog(WIND_TEST_CONTENT_DIR).value;
    wind::items::Inventory bag;
    wind::items::Equipment worn;
    wind::items::QuickSlots quick;
    wind::game::LootStore loot;
    wind::game::GameClock clock{9.0f, 60.0f};

    wind::game::SaveSources sources() { return {progress, bag, worn, quick, loot, clock}; }
};

}  // namespace

TEST(WakePlace, NoCushionWakesAtTheDefaultSpawnOfThisMap) {
    const auto progress = fresh_progress();
    const auto place = wake_place(WIND_TEST_CONTENT_DIR, progress, kMeadow, 120, 45.0f);
    ASSERT_TRUE(place.has_value());
    EXPECT_EQ(place->map, kMeadow);
    const auto expected = wind::engine::tile_feet({6, 12}, kTile);  // test_meadow의 "default" 자리
    EXPECT_FLOAT_EQ(place->feet.x, expected.x);
    EXPECT_FLOAT_EQ(place->feet.y, expected.y);
    EXPECT_EQ(place->hp, 120);    // 가득
    EXPECT_FLOAT_EQ(place->mp, 45.0f);
}

TEST(WakePlace, LastCushionWinsEvenFromAnotherMap) {
    auto progress = fresh_progress();
    progress.rest_at(kMeadow, "meadow_cushion");
    const auto place = wake_place(WIND_TEST_CONTENT_DIR, progress, kCave, 100, 30.0f);  // 동굴에서 쓰러짐
    ASSERT_TRUE(place.has_value());
    EXPECT_EQ(place->map, kMeadow);
    const auto seat = wind::engine::tile_feet({5, 10}, kTile);  // 초원 쿠션 칸
    EXPECT_FLOAT_EQ(place->feet.x, seat.x);
    EXPECT_FLOAT_EQ(place->feet.y, seat.y);
}

TEST(WakePlace, MissingCushionOrMapGivesNothing) {
    auto progress = fresh_progress();
    progress.rest_at(kMeadow, "no_such_cushion");
    EXPECT_FALSE(wake_place(WIND_TEST_CONTENT_DIR, progress, kMeadow, 100, 30.0f).has_value());
    const auto fresh = fresh_progress();
    EXPECT_FALSE(wake_place(WIND_TEST_CONTENT_DIR, fresh, "maps/no_such_map.json", 100, 30.0f).has_value());
}

TEST(WakePlace, RunesLostInAFallStayLostAfterAForcedQuit) {
    const fs::path folder = fs::temp_directory_path() / "wind_wake_place_fall";
    fs::remove_all(folder);
    wind::save::SaveSlots slots(folder);
    std::string error;
    {
        World before;
        before.progress.earn(100);
        before.progress.rest_at(kMeadow, "meadow_cushion");
        // 쉬면서 저장 (룬 100) — 불러와서 되살리려는 "옛 저장"
        auto rested = wind::game::capture_save(before.sources(), {kMeadow, {176.0f, 344.0f}, 100, 30.0f});
        rested.game_version = "test";
        rested.saved_at = 1000;
        ASSERT_TRUE(slots.write(1, SaveKind::Main, rested, error)) << error;
        rested.saved_at = 1001;  // 지역 전환 자동 저장도 룬 100
        ASSERT_TRUE(slots.write(1, SaveKind::Auto, rested, error)) << error;

        // 동굴에서 쓰러짐: 절반을 잃고 흔적 (FieldRunes::step)
        EXPECT_EQ(before.progress.fall(kCave, {64.0f, 96.0f}), 50);
        // 그 박자의 저장 (FieldSession::save_after_fall과 같은 순서): 깨어날 자리로 본·자동 둘 다
        const auto place = wake_place(WIND_TEST_CONTENT_DIR, before.progress, kCave, 100, 30.0f);
        ASSERT_TRUE(place.has_value());
        auto fallen = wind::game::capture_save(before.sources(), *place);
        fallen.game_version = "test";
        fallen.saved_at = 1002;
        ASSERT_TRUE(slots.write(1, SaveKind::Main, fallen, error)) << error;
        ASSERT_TRUE(slots.write(1, SaveKind::Auto, fallen, error)) << error;
    }  // 여기서 강제 종료: 부활 페이드도, 그 뒤 저장도 없어요

    // 본·자동 어느 파일에도 룬 100이 남아 있지 않아요 (자동 저장을 불러와 되살릴 수 없게)
    for (const auto kind : {SaveKind::Main, SaveKind::Auto}) {
        const auto read = slots.read(1, kind);
        ASSERT_EQ(read.status, wind::save::SaveStatus::Ok);
        EXPECT_EQ(read.data->runes, 50);
    }
    // 새로 켜서 불러오기 (SavePicker가 더 새것을 골라요)
    wind::game::SavePicker picker(slots, 1, [](const wind::save::SaveData&, std::vector<std::string>&) { return true; });
    const auto outcome = picker.examine();
    ASSERT_EQ(outcome.start, wind::game::SavePicker::Start::Load);
    World after;
    auto sources = after.sources();
    ASSERT_TRUE(wind::game::apply_save(*outcome.data, sources, after.catalog));
    EXPECT_EQ(after.progress.runes(), 50);                  // 잃은 뒤
    ASSERT_TRUE(after.progress.trace().has_value());        // 새 흔적 (되찾으면 다시 100)
    EXPECT_EQ(after.progress.trace()->amount, 50);
    EXPECT_EQ(after.progress.trace()->map, kCave);
    EXPECT_EQ(outcome.data->position.map, kMeadow);         // 마지막 쿠션에서 깨어나요
    EXPECT_EQ(outcome.data->hp, 100);                  // 가득
    fs::remove_all(folder);
}
