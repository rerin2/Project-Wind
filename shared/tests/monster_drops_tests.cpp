// 이 파일은 무엇을 하나요?
// 저장소의 진짜 몬스터 드롭 표가 진짜 아이템 도감과 맞는지 숙제 검사(테스트)해요. (14b)
// 몬스터 파일은 도감을 모르고 읽혀서, 서로 맞는지는 여기(그리고 게임이 맵을 열 때)에서 확인해요.
#include "wind/content/item_catalog_loader.hpp"
#include "wind/content/monster_loader.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace {

TEST(MonsterDrops, EveryRepositoryDropIsInTheCatalog) {
    const auto catalog = wind::content::load_item_catalog(WIND_TEST_CONTENT_DIR);
    ASSERT_TRUE(catalog.ok());
    int checked = 0;  // 드롭 줄을 몇 개 확인했나 (0이면 아무것도 안 본 것)
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(WIND_TEST_CONTENT_DIR) / "monsters")) {
        if (entry.path().extension() != ".json") continue;
        const std::string path = "monsters/" + entry.path().filename().string();
        const auto monster = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, path);
        ASSERT_TRUE(monster.ok()) << path;
        for (const auto& drop : monster.value->drops) {
            EXPECT_NE(catalog.value->find(drop.item_id), nullptr) << path << " drops unknown item " << drop.item_id;
            ++checked;
        }
    }
    EXPECT_GT(checked, 0);
}

}  // namespace
