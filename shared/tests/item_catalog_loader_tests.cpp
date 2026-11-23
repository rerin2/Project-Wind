// 이 파일은 무엇을 하나요?
// 아이템 도감 파일 검사를 숙제 검사(테스트)해요. (명세 10.1, 14.4, 14a)
#include "wind/content/item_catalog_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::parse_item_catalog;
using wind::items::ItemKind;

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 가짜 무기 확인: "weapons/ok.json"만 읽힌다고 답해요
const wind::content::WeaponExists kFakeWeapons = [](std::string_view path) {
    return path == "weapons/ok.json";
};

json potion() {
    return json{{"id", "hp_potion"}, {"name", "회복 물약"}, {"description", "HP를 채운다."},
                {"kind", "consumable"}, {"max_stack", 20}, {"icon", 0},
                {"use", {{"heal_hp", 50}}}};
}

json jelly() {
    return json{{"id", "jelly"}, {"name", "젤리"}, {"description", "재료"},
                {"kind", "material"}, {"max_stack", 99}, {"icon", 2}, {"rarity", "uncommon"}};
}

json sword() {
    return json{{"id", "sword"}, {"name", "검"}, {"description", "무기"},
                {"kind", "weapon"}, {"max_stack", 1}, {"icon", 6}, {"rarity", "rare"},
                {"weapon", "weapons\\ok.json"}};
}

json make_doc(json items) {
    return json{{"schema_version", 1}, {"items", std::move(items)}};
}

TEST(ItemCatalogLoader, ParsesAllThreeKinds) {
    const auto result = parse_item_catalog(make_doc(json::array({potion(), jelly(), sword()})), "items/c.json", kFakeWeapons);
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& catalog = *result.value;
    ASSERT_EQ(catalog.size(), 3u);
    const auto* p = catalog.find("hp_potion");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->kind, ItemKind::Consumable);
    EXPECT_EQ(p->max_stack, 20);
    ASSERT_TRUE(p->use.has_value());
    EXPECT_EQ(p->use->heal_hp, 50);
    EXPECT_EQ(p->use->heal_mp, 0);
    EXPECT_EQ(p->rarity, wind::items::Rarity::Common);  // rarity가 없으면 common
    EXPECT_FALSE(p->equip.has_value());
    const auto* j = catalog.find("jelly");
    EXPECT_EQ(j->rarity, wind::items::Rarity::Uncommon);
    EXPECT_FALSE(j->use.has_value());
    const auto* s = catalog.find("sword");
    EXPECT_EQ(s->weapon, "weapons/ok.json");  // "\\"는 "/"로 정리돼요
    EXPECT_EQ(s->equip, wind::items::EquipSlot::Weapon);
    EXPECT_EQ(catalog.rank("jelly"), 1u);  // 파일에 적힌 순서
}

TEST(ItemCatalogLoader, RejectsDuplicateIds) {
    const auto result = parse_item_catalog(make_doc(json::array({jelly(), jelly()})), "c.json", kFakeWeapons);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "items[1].id"));
}

TEST(ItemCatalogLoader, RejectsUnknownKindAndRarity) {
    auto a = jelly();
    a["kind"] = "food";
    auto b = jelly();
    b["id"] = "b";
    b["rarity"] = "legendary";
    const auto result = parse_item_catalog(make_doc(json::array({a, b})), "c.json", kFakeWeapons);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "items[0].kind"));
    EXPECT_TRUE(has_error_at(result.errors, "items[1].rarity"));
}

TEST(ItemCatalogLoader, RejectsOutOfRangeNumbers) {
    auto a = jelly();
    a["max_stack"] = 0;  // 1보다 작아요
    auto b = jelly();
    b["id"] = "b";
    b["max_stack"] = 1000;  // 999보다 커요
    auto c = jelly();
    c["id"] = "c";
    c["icon"] = -1;
    const auto result = parse_item_catalog(make_doc(json::array({a, b, c})), "c.json", kFakeWeapons);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "items[0].max_stack"));
    EXPECT_TRUE(has_error_at(result.errors, "items[1].max_stack"));
    EXPECT_TRUE(has_error_at(result.errors, "items[2].icon"));
}

TEST(ItemCatalogLoader, ConsumableNeedsAUsefulUse) {
    auto no_use = potion();
    no_use.erase("use");
    auto empty_use = potion();
    empty_use["id"] = "b";
    empty_use["use"] = json::object();  // 아무것도 안 채우는 물약
    const auto result = parse_item_catalog(make_doc(json::array({no_use, empty_use})), "c.json", kFakeWeapons);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "items[0].use"));
    EXPECT_TRUE(has_error_at(result.errors, "items[1].use"));
}

TEST(ItemCatalogLoader, OnlyMatchingKindsHaveUseOrWeapon) {
    auto a = jelly();
    a["use"] = {{"heal_hp", 5}};  // 재료는 쓸 수 없어요
    auto b = jelly();
    b["id"] = "b";
    b["weapon"] = "weapons/ok.json";  // 재료는 무기가 아니에요
    const auto result = parse_item_catalog(make_doc(json::array({a, b})), "c.json", kFakeWeapons);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "items[0].use"));
    EXPECT_TRUE(has_error_at(result.errors, "items[1].weapon"));
}

TEST(ItemCatalogLoader, WeaponMustPointToALoadableFileAndNotStack) {
    auto missing = sword();
    missing["weapon"] = "weapons/missing.json";  // 참조 검사에서 걸려요
    auto stacked = sword();
    stacked["id"] = "b";
    stacked["max_stack"] = 5;
    auto escape = sword();
    escape["id"] = "c";
    escape["weapon"] = "../secret.json";  // 콘텐츠 폴더 밖
    const auto result = parse_item_catalog(make_doc(json::array({missing, stacked, escape})), "c.json", kFakeWeapons);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "items[0].weapon"));
    EXPECT_TRUE(has_error_at(result.errors, "items[1].max_stack"));
    EXPECT_TRUE(has_error_at(result.errors, "items[2].weapon"));
}

TEST(ItemCatalogLoader, RejectsBadDocumentShapes) {
    EXPECT_FALSE(parse_item_catalog(json::array(), "c.json", kFakeWeapons).ok());
    EXPECT_FALSE(parse_item_catalog(json{{"schema_version", 2}, {"items", json::array({jelly()})}}, "c.json", kFakeWeapons).ok());
    EXPECT_FALSE(parse_item_catalog(make_doc(json::array()), "c.json", kFakeWeapons).ok());  // 빈 도감
    EXPECT_FALSE(parse_item_catalog(make_doc(json::array({42})), "c.json", kFakeWeapons).ok());  // 아이템이 묶음이 아님
}

// 진짜 콘텐츠 폴더의 도감이 검사를 통과해요 (무기 참조도 진짜 파일로 확인)
TEST(ItemCatalogLoader, RealCatalogLoads) {
    const auto result = wind::content::load_item_catalog(WIND_TEST_CONTENT_DIR);
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& catalog = *result.value;
    ASSERT_NE(catalog.find("hp_potion"), nullptr);
    EXPECT_TRUE(catalog.find("hp_potion")->use.has_value());
    ASSERT_NE(catalog.find("longsword"), nullptr);
    EXPECT_EQ(catalog.find("longsword")->weapon, "weapons/longsword.json");
}

}  // namespace
