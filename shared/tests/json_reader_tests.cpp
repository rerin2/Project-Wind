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

}  // namespace
