// 이 파일은 무엇을 하나요?
// 저장 내용 → JSON 쓰기(16a)를 숙제 검사(테스트)해요: 지금 버전 붙이기, 빈 칸은 null, 칸 이름.
#include "wind/save/save_writer.hpp"

#include "sample_save.hpp"

#include <gtest/gtest.h>

namespace {

using wind::save::save_to_json;
using wind::save::test::sample_save;

TEST(SaveWriter, StampsTheCurrentSchemaVersion) {
    const auto doc = save_to_json(sample_save());
    EXPECT_EQ(doc["schema_version"], wind::save::kSaveSchemaVersion);
    EXPECT_EQ(doc["game_version"], "0.1.0");
}

TEST(SaveWriter, WritesEmptySlotsAsNull) {
    const auto doc = save_to_json(sample_save());
    EXPECT_TRUE(doc["spells"]["slots"][1].is_null());
    EXPECT_EQ(doc["spells"]["slots"][2], "moon_blade");
    EXPECT_TRUE(doc["inventory"][1].is_null());
    EXPECT_EQ(doc["inventory"][0]["item"], "red_potion");
    EXPECT_EQ(doc["equipment"]["weapon"], "longsword");
    EXPECT_TRUE(doc["equipment"]["head"].is_null());
    EXPECT_TRUE(doc["quick_slots"][0].is_null());
}

TEST(SaveWriter, WritesNoTraceOrCushionAsNull) {
    auto data = sample_save();
    data.rune_trace.reset();
    data.last_cushion.reset();
    const auto doc = save_to_json(data);
    EXPECT_TRUE(doc["hero"]["rune_trace"].is_null());
    EXPECT_TRUE(doc["hero"]["last_cushion"].is_null());
}

TEST(SaveWriter, UsesStatNames) {
    const auto doc = save_to_json(sample_save());
    EXPECT_EQ(doc["hero"]["stats"]["vigor"], 12);
    EXPECT_EQ(doc["hero"]["stats"]["intelligence"], 20);
    EXPECT_EQ(doc["hero"]["stats"]["mind"], 10);
}

}  // namespace
