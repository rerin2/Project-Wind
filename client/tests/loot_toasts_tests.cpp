// 이 파일은 무엇을 하나요?
// 왼쪽 아래 아이템 알림 줄(글, 합치기, 최대 줄 수, 사라지기)을 숙제 검사(테스트)해요. (14b)
#include "game/ui/loot_toasts.hpp"

#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::LootNotice;
using wind::game::LootToasts;

wind::items::ItemCatalog make_catalog() {
    wind::items::ItemCatalog catalog;
    wind::items::ItemDef jelly;
    jelly.id = "jelly";
    jelly.name = "슬라임 젤리";
    jelly.icon = 2;
    catalog.add(jelly);
    wind::items::ItemDef sword;
    sword.id = "sword";
    sword.name = "롱소드";
    sword.icon = 6;
    catalog.add(sword);
    return catalog;
}

TEST(LootToasts, PickupShowsNameCountAndIcon) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    toasts.push({{"jelly", 2}, false}, catalog);
    ASSERT_EQ(toasts.lines().size(), 1u);
    EXPECT_EQ(toasts.lines()[0].text, "슬라임 젤리 ×2");
    EXPECT_EQ(toasts.lines()[0].icon, 2);
    EXPECT_FALSE(toasts.lines()[0].bag_full);
}

TEST(LootToasts, SameItemSoonAfterAddsUp) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    toasts.push({{"jelly", 1}, false}, catalog);
    toasts.update(0.5f);
    toasts.push({{"jelly", 2}, false}, catalog);  // 1초 안: 합쳐요
    ASSERT_EQ(toasts.lines().size(), 1u);
    EXPECT_EQ(toasts.lines()[0].text, "슬라임 젤리 ×3");
    toasts.update(1.2f);
    toasts.push({{"jelly", 1}, false}, catalog);  // 1초 넘음: 새 줄
    EXPECT_EQ(toasts.lines().size(), 2u);
}

TEST(LootToasts, BagFullIsItsOwnWarningLine) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    toasts.push({{"sword", 1}, false}, catalog);
    toasts.push({{"sword", 1}, true}, catalog);
    ASSERT_EQ(toasts.lines().size(), 2u);
    EXPECT_TRUE(toasts.lines()[1].bag_full);
    EXPECT_EQ(toasts.lines()[1].text, "가방이 가득 찼다 · 롱소드");
}

TEST(LootToasts, KeepsFourNewestLinesAndFadesThemOut) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    for (int i = 0; i < 6; ++i) toasts.push({{i % 2 == 0 ? "jelly" : "sword", 1}, false}, catalog);
    EXPECT_EQ(toasts.lines().size(), LootToasts::kMaxLines);
    EXPECT_FLOAT_EQ(LootToasts::slide(toasts.lines()[0]), 0.0f);  // 막 나타남: 아직 왼쪽 밖
    toasts.update(1.0f);
    EXPECT_FLOAT_EQ(LootToasts::slide(toasts.lines()[0]), 1.0f);
    EXPECT_FLOAT_EQ(LootToasts::alpha(toasts.lines()[0]), 1.0f);
    toasts.update(1.4f);  // 2.4초: 흐려지는 중
    EXPECT_LT(LootToasts::alpha(toasts.lines()[0]), 1.0f);
    toasts.update(0.3f);  // 2.7초: 사라짐
    EXPECT_TRUE(toasts.lines().empty());
}

TEST(LootToasts, UnknownItemShowsItsId) {
    const wind::items::ItemCatalog catalog;
    LootToasts toasts;
    toasts.push({{"mystery", 1}, false}, catalog);
    EXPECT_EQ(toasts.lines()[0].text, "mystery ×1");
}

}  // namespace
