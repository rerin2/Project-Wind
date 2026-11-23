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
    toasts.push({{"jelly", 2}, LootNotice::Kind::PickedUp}, catalog);
    ASSERT_EQ(toasts.lines().size(), 1u);
    EXPECT_EQ(toasts.lines()[0].text, "슬라임 젤리 ×2");
    EXPECT_EQ(toasts.lines()[0].icon, 2);
    EXPECT_FALSE(toasts.lines()[0].warning());
}

TEST(LootToasts, SameItemSoonAfterAddsUp) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    toasts.push({{"jelly", 1}, LootNotice::Kind::PickedUp}, catalog);
    toasts.update(0.5f);
    toasts.push({{"jelly", 2}, LootNotice::Kind::PickedUp}, catalog);  // 1초 안: 합쳐요
    ASSERT_EQ(toasts.lines().size(), 1u);
    EXPECT_EQ(toasts.lines()[0].text, "슬라임 젤리 ×3");
    toasts.update(1.2f);
    toasts.push({{"jelly", 1}, LootNotice::Kind::PickedUp}, catalog);  // 1초 넘음: 새 줄
    EXPECT_EQ(toasts.lines().size(), 2u);
}

TEST(LootToasts, BagFullIsItsOwnWarningLine) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    toasts.push({{"sword", 1}, LootNotice::Kind::PickedUp}, catalog);
    toasts.push({{"sword", 1}, LootNotice::Kind::BagFull}, catalog);
    ASSERT_EQ(toasts.lines().size(), 2u);
    EXPECT_TRUE(toasts.lines()[1].warning());
    EXPECT_EQ(toasts.lines()[1].text, "가방이 가득 찼다 · 롱소드");
}

TEST(LootToasts, KeepsFourNewestLinesAndFadesThemOut) {
    const auto catalog = make_catalog();
    LootToasts toasts;
    for (int i = 0; i < 6; ++i) toasts.push({{i % 2 == 0 ? "jelly" : "sword", 1}, LootNotice::Kind::PickedUp}, catalog);
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

// 14d: 물약 알림 두 가지는 빨간 경고 줄이고, 같은 경고가 1초 안에 또 오면 줄이 늘지 않아요
TEST(LootToasts, PotionWarningsAndRepeatedWarningsMerge) {
    auto catalog = make_catalog();
    wind::items::ItemDef potion;
    potion.id = "potion";
    potion.name = "회복 물약";
    catalog.add(potion);
    LootToasts toasts;
    toasts.push({{"potion", 1}, LootNotice::Kind::AlreadyFull}, catalog);
    toasts.push({{"potion", 1}, LootNotice::Kind::AlreadyFull}, catalog);  // 다시 눌러도 한 줄
    toasts.push({{"potion", 1}, LootNotice::Kind::CannotUse}, catalog);
    ASSERT_EQ(toasts.lines().size(), 2u);
    EXPECT_EQ(toasts.lines()[0].text, "이미 가득하다 · 회복 물약");
    EXPECT_EQ(toasts.lines()[1].text, "지금은 마실 수 없다");
    EXPECT_TRUE(toasts.lines()[0].warning());
    EXPECT_TRUE(toasts.lines()[1].warning());
}

TEST(LootToasts, UnknownItemShowsItsId) {
    const wind::items::ItemCatalog catalog;
    LootToasts toasts;
    toasts.push({{"mystery", 1}, LootNotice::Kind::PickedUp}, catalog);
    EXPECT_EQ(toasts.lines()[0].text, "mystery ×1");
}

}  // namespace
