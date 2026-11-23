// 이 파일은 무엇을 하나요?
// 아이템 도감(넣기·찾기·순서)을 숙제 검사(테스트)해요. (14a)
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::items::ItemCatalog;
using wind::items::ItemDef;

ItemDef make_def(std::string id) {
    ItemDef def;
    def.id = std::move(id);
    def.name = "이름";
    return def;
}

TEST(ItemCatalog, FindsAddedItemsAndRejectsDuplicates) {
    ItemCatalog catalog;
    EXPECT_TRUE(catalog.add(make_def("a")));
    EXPECT_TRUE(catalog.add(make_def("b")));
    EXPECT_FALSE(catalog.add(make_def("a")));  // 같은 id는 두 번 못 넣어요
    EXPECT_EQ(catalog.size(), 2u);
    ASSERT_NE(catalog.find("b"), nullptr);
    EXPECT_EQ(catalog.find("b")->id, "b");
    EXPECT_EQ(catalog.find("zzz"), nullptr);  // 모르는 아이템
}

TEST(ItemCatalog, RankFollowsInsertOrderAndUnknownGoesLast) {
    ItemCatalog catalog;
    catalog.add(make_def("z"));
    catalog.add(make_def("a"));
    EXPECT_EQ(catalog.rank("z"), 0u);  // 먼저 적힌 것이 앞 (알파벳 순서가 아니에요)
    EXPECT_EQ(catalog.rank("a"), 1u);
    EXPECT_EQ(catalog.rank("nope"), 2u);  // 없으면 맨 뒤
    ASSERT_EQ(catalog.order().size(), 2u);
    EXPECT_EQ(catalog.order()[0], "z");
}

}  // namespace
