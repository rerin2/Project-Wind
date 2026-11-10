// 이 파일은 무엇을 하나요?
// 하루 색 곡선 파일 검사가 맞게 동작하는지 숙제 검사(테스트)를 해요.
#include "wind/content/day_cycle_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::parse_day_cycle;

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

json make_doc() {
    return json{{"schema_version", 1},
                {"keys", {{{"hour", 0}, {"ambient", "#2a3060"}, {"night", 1}},
                          {{"hour", 12}, {"ambient", "#ffffff"}, {"night", 0}}}}};
}

TEST(DayCycleLoader, ParsesValidCycle) {
    const auto result = parse_day_cycle(make_doc(), "time/day.json");
    ASSERT_TRUE(result.ok());
    ASSERT_EQ(result.value->keys.size(), 2u);
    EXPECT_FLOAT_EQ(result.value->keys[1].hour, 12.0f);
}

// 시각이 거꾸로이거나 24시, 밤 정도가 1 초과, 점이 하나뿐이면 거절해요.
TEST(DayCycleLoader, RejectsBadKeys) {
    auto backwards = make_doc();
    backwards["keys"][1]["hour"] = 0;
    EXPECT_TRUE(has_error_at(parse_day_cycle(backwards, "t.json").errors, "keys[1].hour"));

    auto too_late = make_doc();
    too_late["keys"][1]["hour"] = 24;
    EXPECT_TRUE(has_error_at(parse_day_cycle(too_late, "t.json").errors, "keys[1].hour"));

    auto too_dark = make_doc();
    too_dark["keys"][0]["night"] = 2;
    EXPECT_TRUE(has_error_at(parse_day_cycle(too_dark, "t.json").errors, "keys[0].night"));

    auto single = make_doc();
    single["keys"].erase(1);
    EXPECT_TRUE(has_error_at(parse_day_cycle(single, "t.json").errors, "keys"));
}

// 저장소의 진짜 하루 색 곡선도 통과해야 해요.
TEST(DayCycleLoader, RepositoryDayCycleIsValid) {
    const auto result = wind::content::load_day_cycle_file(WIND_TEST_CONTENT_DIR, "time/day_cycle.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_GE(result.value->keys.size(), 6u);
}

}  // namespace
