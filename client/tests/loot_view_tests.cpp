// 이 파일은 무엇을 하나요?
// 바닥 아이템 연출(톡 튀는 높이, 떨어지는 길, 빨려 들기, 밤 빛)을 숙제 검사(테스트)해요. (14b)
#include "game/presentation/loot_view.hpp"

#include "game/items/map_loot.hpp"

#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::GroundItem;
using wind::game::LootView;

TEST(LootView, HopRisesFallsBouncesThenRests) {
    EXPECT_FLOAT_EQ(LootView::hop_height(0.0f), 0.0f);    // 쓰러진 자리 땅에서 시작
    EXPECT_NEAR(LootView::hop_height(0.225f), 14.0f, 0.01f);  // 첫 포물선 꼭대기
    EXPECT_NEAR(LootView::hop_height(0.45f), 0.0f, 0.01f);    // 땅에 닿고
    EXPECT_GT(LootView::hop_height(0.52f), 0.0f);             // 작게 한 번 더
    EXPECT_LT(LootView::hop_height(0.52f), 4.01f);
    EXPECT_FLOAT_EQ(LootView::hop_height(0.7f), 0.0f);        // 그다음은 바닥
}

TEST(LootView, HopMovesFromMonsterToLandingSpot) {
    GroundItem item;
    item.from = {100.0f, 100.0f};
    item.at = {120.0f, 110.0f};
    item.age = 0.0f;
    EXPECT_FLOAT_EQ(LootView::hop_ground(item).x, 100.0f);
    item.age = 0.2f;
    const auto mid = LootView::hop_ground(item);
    EXPECT_GT(mid.x, 100.0f);
    EXPECT_LT(mid.x, 120.0f);
    item.age = 5.0f;  // 다 떨어진 뒤 (맵에 다시 들어왔을 때도)
    EXPECT_FLOAT_EQ(LootView::hop_ground(item).x, 120.0f);
    EXPECT_FLOAT_EQ(LootView::hop_ground(item).y, 110.0f);
}

TEST(LootView, PickedIconFliesInAndDisappears) {
    const wind::items::ItemCatalog catalog;
    LootView view({}, catalog);
    GroundItem picked;
    picked.stack = {"jelly", 1};
    picked.at = {50.0f, 50.0f};
    view.collect({picked});
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(16);
    view.update(0.01f, {0.0f, 0.0f}, random, pool);
    EXPECT_EQ(view.flying(), 1u);  // 빨려 드는 중
    for (int i = 0; i < 40; ++i) view.update(0.01f, {0.0f, 0.0f}, random, pool);
    EXPECT_EQ(view.flying(), 0u);  // 0.28초 뒤 몸에 닿아 사라져요
}

TEST(LootView, EachGroundItemGlowsInItsRarityColor) {
    wind::items::ItemCatalog catalog;
    wind::items::ItemDef rare;
    rare.id = "sword";
    rare.rarity = wind::items::Rarity::Rare;
    catalog.add(rare);
    LootView view({}, catalog);
    wind::game::MapLoot loot;
    loot.drop({"sword", 1}, {}, {10.0f, 10.0f});
    loot.drop({"unknown", 1}, {}, {30.0f, 10.0f});  // 도감에 없어도 멈추지 않아요 (보통 색)
    const auto lights = view.lights(loot);
    ASSERT_EQ(lights.size(), 2u);
    EXPECT_NE(lights[0].color.g, lights[1].color.g);  // 희귀(분홍빛 보라)와 보통(따뜻한 흰색)은 색이 달라요
}

}  // namespace
