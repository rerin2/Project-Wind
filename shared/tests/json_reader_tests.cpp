// 이 파일은 무엇을 하나요?
// 설명서 읽기 도우미(JsonReader)의 숫자·원래 값 읽기를 숙제 검사(테스트)해요.
#include "wind/content/json_reader.hpp"

#include <gtest/gtest.h>

namespace {

using nlohmann::json;
using wind::content::ErrorList;
using wind::content::JsonReader;

// 소수와 정수 모두 숫자로 읽고, 범위를 벗어나거나 글자면 거절해요.
TEST(JsonReader, ReadsDecimalNumbersWithRange) {
    const json doc{{"a", 0.35}, {"b", 3}, {"c", 12.5}, {"d", "3"}};
    ErrorList errors;
    JsonReader reader(doc, "unit.json", "", errors);
    EXPECT_DOUBLE_EQ(*reader.number("a", 0.0, 1.0), 0.35);
    EXPECT_DOUBLE_EQ(*reader.number("b", 0.0, 10.0), 3.0);
    EXPECT_FALSE(reader.number("c", 0.0, 10.0).has_value());
    EXPECT_FALSE(reader.number("d", 0.0, 10.0).has_value());
    EXPECT_FALSE(reader.number("missing", 0.0, 10.0).has_value());
    ASSERT_EQ(errors.items().size(), 3u);
    EXPECT_EQ(errors.items()[0].field, "c");
    EXPECT_EQ(errors.items()[1].field, "d");
    EXPECT_EQ(errors.items()[2].field, "missing");
}

// raw는 모양 검사 없이 꺼내고, 없으면 "없어요" 오류를 적어요.
TEST(JsonReader, RawReturnsValueOrReportsMissing) {
    const json doc{{"range", {1, 2}}};
    ErrorList errors;
    JsonReader reader(doc, "unit.json", "", errors);
    const auto* range = reader.raw("range");
    ASSERT_NE(range, nullptr);
    EXPECT_TRUE(range->is_array());
    EXPECT_EQ(reader.raw("nope"), nullptr);
    EXPECT_EQ(errors.items().size(), 1u);
}

// 범위·색·파일 위치 읽기: 맞으면 값, 틀리면 칸 주소와 함께 오류를 적어요.
TEST(JsonReader, ReadsRangeColorAndContentPath) {
    const json doc{{"r1", 3}, {"r2", {2, 5}}, {"r3", {5, 2}}, {"c1", "#ff8000"}, {"c2", "orange"},
                   {"p1", "fx\\rain.json"}, {"p2", "../up.json"}};
    ErrorList errors;
    JsonReader reader(doc, "unit.json", "", errors);
    const auto r1 = reader.range("r1", 0.0, 10.0);
    ASSERT_TRUE(r1.has_value());
    EXPECT_DOUBLE_EQ(r1->min, 3.0);
    EXPECT_DOUBLE_EQ(r1->max, 3.0);
    const auto r2 = reader.range("r2", 0.0, 10.0);
    ASSERT_TRUE(r2.has_value());
    EXPECT_DOUBLE_EQ(r2->max, 5.0);
    EXPECT_FALSE(reader.range("r3", 0.0, 10.0).has_value());  // 최소 > 최대
    EXPECT_EQ(reader.color("c1"), (wind::world::Rgba{255, 128, 0, 255}));
    EXPECT_FALSE(reader.color("c2").has_value());
    EXPECT_EQ(reader.content_path("p1"), std::optional<std::string>("fx/rain.json"));
    EXPECT_FALSE(reader.content_path("p2").has_value());
    ASSERT_EQ(errors.items().size(), 3u);
    EXPECT_EQ(errors.items()[0].field, "r3");
    EXPECT_EQ(errors.items()[1].field, "c2");
    EXPECT_EQ(errors.items()[2].field, "p2");
}

}  // namespace
