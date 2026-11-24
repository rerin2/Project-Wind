// 이 파일은 무엇을 하나요?
// 게임 상태 ⇄ 저장 모양(16b)을 숙제 검사(테스트)해요.
// 16단계 완료 기준 "저장 → 종료 → 불러오기로 위치·능력치·룬·마지막 쿠션·마술·인벤토리·장비가 그대로 복구됨"을
// 규칙 쪽에서 확인해요: 상태를 꾸며 저장 파일로 쓰고, 새 게임 상태에 읽어 되살린 뒤 다시 모으면 같아요.
#include "game/save/save_apply.hpp"

#include "game/items/field_loot.hpp"
#include "game/items/loot_store.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/save/save_capture.hpp"
#include "game/world/game_clock.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/items/equipment.hpp"
#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/items/quick_slots.hpp"
#include "wind/save/save_slots.hpp"

#include <gtest/gtest.h>

#include <filesystem>

namespace {

using wind::progression::StatId;

// 필드 밖에 있는 게임 상태 한 벌 (앱의 ClientApp·ItemSetup과 같은 구성)
struct World {
    wind::game::HeroProgress progress = *wind::game::HeroProgress::load(WIND_TEST_CONTENT_DIR);
    wind::items::ItemCatalog catalog = *wind::content::load_item_catalog(WIND_TEST_CONTENT_DIR).value;
    wind::items::Inventory bag;
    wind::items::Equipment worn;
    wind::items::QuickSlots quick;
    wind::game::LootStore loot;
    wind::game::GameClock clock{9.0f, 60.0f};

    wind::game::SaveSources sources() { return {progress, bag, worn, quick, loot, clock}; }
};

const wind::game::HeroPlace kHere{"maps/test_meadow.json", {208.0f, 408.0f}, 77, 31.5f};

// 여러 가지를 해 둔 게임: 레벨업, 쉬기, 쓰러져 흔적, 마술, 가방, 장비, 단축 칸, 바닥 아이템, 시간
void play_a_while(World& w) {
    w.progress.earn(100000);
    auto target = w.progress.stats();
    target[StatId::Intelligence] = 16;
    target[StatId::Strength] = 12;
    ASSERT_TRUE(w.progress.try_raise(target));
    w.progress.rest_at("maps/test_meadow.json", "meadow_cushion");
    w.progress.fall("maps/test_meadow.json", {300.0f, 410.0f});  // 룬 절반이 흔적으로
    auto& book = w.progress.spells();
    book.learn("star_shard");
    book.learn("moon_blade");
    book.put(0, "star_shard", 10, 16, 3);
    book.put(2, "moon_blade", 14, 16, 3);
    (void)book.select_next(3);
    w.bag.try_add(w.catalog, "hp_potion", 3);
    w.bag.try_add(w.catalog, "dagger", 1);
    w.worn.wear(wind::items::EquipSlot::Weapon, "longsword");
    w.quick.assign(1, "hp_potion", w.catalog);
    w.loot.on("maps/test_cave.json").drop({"slime_jelly", 2}, {10.0f, 10.0f}, {64.0f, 96.0f});
    w.clock.skip_hours(30.0f);
}

TEST(SaveApply, CaptureThenApplyRestoresEverything) {
    World before;
    play_a_while(before);
    const auto data = wind::game::capture_save(before.sources(), kHere);

    World after;  // 갓 켠 게임
    auto sources = after.sources();
    ASSERT_TRUE(wind::game::apply_save(data, sources, after.catalog));
    EXPECT_EQ(wind::game::capture_save(after.sources(), kHere), data);
    // 몇 가지는 직접 확인해요
    EXPECT_EQ(after.progress.level(), before.progress.level());
    EXPECT_EQ(after.progress.runes(), before.progress.runes());
    EXPECT_EQ(after.progress.last_cushion()->id, "meadow_cushion");
    EXPECT_EQ(after.progress.trace()->amount, before.progress.trace()->amount);
    EXPECT_EQ(after.progress.spells().selected(3), "moon_blade");
    EXPECT_EQ(after.worn.worn(wind::items::EquipSlot::Weapon), "longsword");
    EXPECT_EQ(after.quick.at(1), "hp_potion");
    EXPECT_EQ(after.bag.at(0)->count, 3);
    EXPECT_EQ(after.clock.day(), 1);
    // 바닥 아이템은 다 떨어진 상태라 바로 주울 수 있어요
    const auto& ground = after.loot.on("maps/test_cave.json").items();
    ASSERT_EQ(ground.size(), 1u);
    EXPECT_GE(ground[0].age, wind::game::FieldLoot::kSettleSeconds);
}

TEST(SaveApply, SurvivesTheSaveFileOnDisk) {
    // 저장 → (게임 끄기) → 불러오기: 파일을 거쳐도 같아요
    const auto folder = std::filesystem::temp_directory_path() / "wind_save_apply_disk";
    std::filesystem::remove_all(folder);
    World before;
    play_a_while(before);
    auto data = wind::game::capture_save(before.sources(), kHere);
    data.game_version = "test";
    wind::save::SaveSlots slots(folder);
    std::string error;
    ASSERT_TRUE(slots.write(1, wind::save::SaveKind::Main, data, error)) << error;

    const auto read = slots.read(1, wind::save::SaveKind::Main);
    ASSERT_EQ(read.status, wind::save::SaveStatus::Ok);
    World after;
    auto sources = after.sources();
    ASSERT_TRUE(wind::game::apply_save(*read.data, sources, after.catalog));
    auto again = wind::game::capture_save(after.sources(), kHere);
    again.game_version = "test";
    EXPECT_EQ(again, data);
    std::filesystem::remove_all(folder);
}

TEST(SaveApply, BadDataChangesNothing) {
    World before;
    play_a_while(before);
    auto data = wind::game::capture_save(before.sources(), kHere);
    data.quick_slots[0] = "slime_jelly";  // 쓸 수 없는 아이템: 단축 칸에 못 넣어요

    World after;
    const auto untouched = wind::game::capture_save(after.sources(), kHere);
    auto sources = after.sources();
    EXPECT_FALSE(wind::game::apply_save(data, sources, after.catalog));
    EXPECT_EQ(wind::game::capture_save(after.sources(), kHere), untouched);
}

TEST(SaveApply, CaptureSkipsMapsWithoutGroundItems) {
    World w;
    (void)w.loot.on("maps/test_forest.json");  // 들어가 봤지만 아무것도 없는 맵
    EXPECT_TRUE(wind::game::capture_save(w.sources(), kHere).ground.empty());
}

}  // namespace
