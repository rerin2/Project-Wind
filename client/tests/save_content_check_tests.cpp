// 이 파일은 무엇을 하나요?
// 저장 ↔ 진짜 콘텐츠 확인(16b)을 숙제 검사(테스트)해요.
// 올바른 저장은 통과하고, 없는 아이템·마술·맵·쿠션, 벽 속 위치, 지능 모자람, 맞지 않는 장비는 거절해요.
#include "game/save/save_content_check.hpp"

#include "engine/map/coordinates.hpp"
#include "game/combat/spell_shelf.hpp"
#include "game/items/weapon_rack.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/content/map_loader.hpp"
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::save::SaveData;

struct Content {
    wind::items::ItemCatalog catalog = *wind::content::load_item_catalog(WIND_TEST_CONTENT_DIR).value;
    wind::game::WeaponRack weapons = wind::game::WeaponRack::load(WIND_TEST_CONTENT_DIR, catalog);
    wind::game::SpellShelf spells;

    bool check(const SaveData& data, std::vector<std::string>& problems) {
        return wind::game::check_save_content(data, {WIND_TEST_CONTENT_DIR, catalog, weapons, spells}, problems);
    }
    bool check(const SaveData& data) {
        std::vector<std::string> problems;
        return check(data, problems);
    }
};

// 진짜 콘텐츠와 맞는 저장 (초원 기본 자리, 쿠션, 물약, 롱소드, 별빛 조각)
SaveData good_save() {
    SaveData d;
    d.position = {"maps/test_meadow.json", 208.0f, 408.0f};  // 6,12 칸 (기본 등장 자리)
    d.last_cushion = wind::save::SavedCushion{"maps/test_meadow.json", "meadow_cushion"};
    d.discovered_cushions = {"meadow_cushion"};
    d.learned_spells = {"star_shard"};
    d.spell_slots = {"star_shard", "", "", ""};
    d.bag.resize(24);
    d.bag[0] = wind::items::ItemStack{"hp_potion", 3};
    d.equipment = {"longsword", "", "", ""};
    d.quick_slots = {"hp_potion", "", "", ""};
    d.ground = {{"maps/test_cave.json", {{{"slime_jelly", 1}, 64.0f, 64.0f}}}};
    return d;
}

TEST(SaveContentCheck, AcceptsAMatchingSaveAndShelvesItsSpells) {
    Content content;
    std::vector<std::string> problems;
    EXPECT_TRUE(content.check(good_save(), problems)) << (problems.empty() ? "" : problems.front());
    EXPECT_NE(content.spells.find("star_shard"), nullptr);  // 불러온 뒤 바로 쓸 수 있게 올려 둬요
}

TEST(SaveContentCheck, RejectsUnknownItems) {
    Content content;
    auto data = good_save();
    data.bag[3] = wind::items::ItemStack{"golden_apple", 1};
    std::vector<std::string> problems;
    EXPECT_FALSE(content.check(data, problems));
    ASSERT_FALSE(problems.empty());
    EXPECT_NE(problems.front().find("golden_apple"), std::string::npos);
}

TEST(SaveContentCheck, RejectsStacksAboveTheLimit) {
    Content content;
    auto data = good_save();
    data.bag[0]->count = 21;  // 물약은 20개까지
    EXPECT_FALSE(content.check(data));
}

TEST(SaveContentCheck, RejectsEquipmentInTheWrongSlot) {
    Content content;
    auto data = good_save();
    data.equipment[0] = "hp_potion";
    EXPECT_FALSE(content.check(data));
}

TEST(SaveContentCheck, RejectsUnusableQuickSlotItems) {
    Content content;
    auto data = good_save();
    data.quick_slots[1] = "slime_jelly";
    EXPECT_FALSE(content.check(data));
}

TEST(SaveContentCheck, RejectsUnknownSpellsAndLowIntelligence) {
    Content content;
    auto data = good_save();
    data.learned_spells.push_back("fireball");
    EXPECT_FALSE(content.check(data));

    data = good_save();
    data.learned_spells.push_back("moon_blade");
    data.spell_slots[1] = "moon_blade";  // 지능 14 필요, 지금 10
    EXPECT_FALSE(content.check(data));
    data.stats[wind::progression::StatId::Intelligence] = 14;
    EXPECT_TRUE(content.check(data));
}

TEST(SaveContentCheck, RejectsMissingMapsAndCushions) {
    Content content;
    auto data = good_save();
    data.position.map = "maps/nowhere.json";
    EXPECT_FALSE(content.check(data));

    data = good_save();
    data.last_cushion->id = "cave_cushion_typo";
    data.discovered_cushions = {"cave_cushion_typo"};
    EXPECT_FALSE(content.check(data));

    data = good_save();
    data.rune_trace = wind::save::SavedTrace{"maps/nowhere.json", 1.0f, 1.0f, 5};
    EXPECT_FALSE(content.check(data));
}

TEST(SaveContentCheck, RejectsPositionInsideAWall) {
    Content content;
    // 초원에서 걸을 수 없는 칸 하나를 찾아 그 칸의 발 자리에 둬요
    const auto map = wind::content::load_map_file(WIND_TEST_CONTENT_DIR, "maps/test_meadow.json");
    ASSERT_TRUE(map.ok());
    const auto& data_map = map.value->map;
    std::optional<wind::world::TileCoord> wall;
    for (int y = 0; y < data_map.height && !wall; ++y) {
        for (int x = 0; x < data_map.width && !wall; ++x) {
            if (!data_map.is_walkable({x, y})) wall = wind::world::TileCoord{x, y};
        }
    }
    ASSERT_TRUE(wall.has_value());
    auto data = good_save();
    const auto feet = wind::engine::tile_feet(*wall, data_map.tile_size);
    data.position.x = feet.x;
    data.position.y = feet.y;
    EXPECT_FALSE(content.check(data));
}

}  // namespace
