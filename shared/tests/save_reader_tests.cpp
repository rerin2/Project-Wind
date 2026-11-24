// 이 파일은 무엇을 하나요?
// 저장 파일 JSON 읽기·검사(16a)를 숙제 검사(테스트)해요:
// 쓰고 읽으면 그대로(왕복), 틀린 칸·범위·겹침·폴더 밖 맵은 통째로 거절, 이상한 JSON에서도 멈추지 않음.
#include "wind/save/save_reader.hpp"

#include "wind/save/save_writer.hpp"

#include "sample_save.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using nlohmann::json;
using wind::content::ErrorList;
using wind::save::save_from_json;
using wind::save::save_to_json;
using wind::save::test::sample_save;

// doc을 읽어 거절되는지 보고, 오류 메시지 중 하나에 field가 들어 있는지 봐요
void expect_rejected(const json& doc, const std::string& field) {
    ErrorList errors;
    EXPECT_FALSE(save_from_json(doc, "slot_1.json", errors).has_value()) << field;
    bool found = false;
    for (const auto& e : errors.items()) {
        found = found || e.field.find(field) != std::string::npos;
    }
    EXPECT_TRUE(found) << "no error mentions " << field;
}

TEST(SaveReader, RoundTripKeepsEverything) {
    const auto data = sample_save();
    ErrorList errors;
    const auto back = save_from_json(save_to_json(data), "slot_1.json", errors);
    ASSERT_TRUE(back.has_value()) << (errors.empty() ? "" : wind::content::to_string(errors.items().front()));
    EXPECT_EQ(*back, data);
}

TEST(SaveReader, RoundTripSurvivesTextDumpAndParse) {
    // 실제 파일처럼 글자로 바꿨다가 다시 읽어도 소수(208.25, 42.5)까지 같아요
    const auto data = sample_save();
    const auto text = save_to_json(data).dump(2);
    ErrorList errors;
    const auto back = save_from_json(json::parse(text), "slot_1.json", errors);
    ASSERT_TRUE(back.has_value());
    EXPECT_EQ(*back, data);
}

TEST(SaveReader, EmptyTraceAndCushionRoundTrip) {
    auto data = sample_save();
    data.rune_trace.reset();
    data.last_cushion.reset();
    data.discovered_cushions.clear();
    ErrorList errors;
    const auto back = save_from_json(save_to_json(data), "slot_1.json", errors);
    ASSERT_TRUE(back.has_value());
    EXPECT_EQ(*back, data);
}

TEST(SaveReader, RejectsWrongSchemaVersion) {
    auto doc = save_to_json(sample_save());
    doc["schema_version"] = wind::save::kSaveSchemaVersion + 1;
    expect_rejected(doc, "schema_version");
}

TEST(SaveReader, RejectsStatsOutsideRange) {
    auto doc = save_to_json(sample_save());
    doc["hero"]["stats"]["strength"] = 61;
    expect_rejected(doc, "hero.stats.strength");
}

TEST(SaveReader, RejectsRunesAboveTheWalletCap) {
    auto doc = save_to_json(sample_save());
    doc["hero"]["runes"] = 1'000'000'000;
    expect_rejected(doc, "hero.runes");
}

TEST(SaveReader, RejectsMapPathsOutsideTheContentFolder) {
    auto doc = save_to_json(sample_save());
    doc["hero"]["position"]["map"] = "../../secret.json";
    expect_rejected(doc, "hero.position.map");
}

TEST(SaveReader, RejectsSlottedSpellThatIsNotLearned) {
    auto doc = save_to_json(sample_save());
    doc["spells"]["slots"][3] = "comet_beam";
    expect_rejected(doc, "spells.slots[3]");
}

TEST(SaveReader, RejectsSameSpellInTwoSlots) {
    auto doc = save_to_json(sample_save());
    doc["spells"]["slots"][1] = "star_shard";
    expect_rejected(doc, "spells.slots[1]");
}

TEST(SaveReader, RejectsWrongSlotCount) {
    auto doc = save_to_json(sample_save());
    doc["quick_slots"] = json::array({nullptr, nullptr});
    expect_rejected(doc, "quick_slots");
}

TEST(SaveReader, RejectsEmptyStringInsteadOfNull) {
    auto doc = save_to_json(sample_save());
    doc["equipment"]["head"] = "";
    expect_rejected(doc, "equipment.head");
}

TEST(SaveReader, RejectsBadStackCounts) {
    auto doc = save_to_json(sample_save());
    doc["inventory"][0]["count"] = 0;
    expect_rejected(doc, "inventory[0].count");
}

TEST(SaveReader, RejectsTooManyBagSlots) {
    auto doc = save_to_json(sample_save());
    doc["inventory"] = json::array();
    for (int i = 0; i < 257; ++i) {
        doc["inventory"].push_back(nullptr);
    }
    expect_rejected(doc, "inventory");
}

TEST(SaveReader, RejectsLastCushionThatWasNeverDiscovered) {
    auto doc = save_to_json(sample_save());
    doc["hero"]["discovered_cushions"] = json::array({"cave_cushion"});
    expect_rejected(doc, "hero.last_cushion.id");
}

TEST(SaveReader, RejectsDuplicateGroundMaps) {
    auto doc = save_to_json(sample_save());
    doc["ground_items"].push_back(doc["ground_items"][0]);
    expect_rejected(doc, "ground_items[1].map");
}

TEST(SaveReader, RejectsHourOf24) {
    auto doc = save_to_json(sample_save());
    doc["clock"]["hour"] = 24.0;
    expect_rejected(doc, "clock.hour");
}

TEST(SaveReader, RejectsMissingSections) {
    auto doc = save_to_json(sample_save());
    doc.erase("hero");
    expect_rejected(doc, "hero");
}

TEST(SaveReader, SurvivesWrongTypesEverywhere) {
    // 모든 칸을 엉뚱한 모양으로 바꿔도 멈추지 않고 거절해요
    const auto base = save_to_json(sample_save());
    for (const auto& [key, value] : base.items()) {
        if (key == "schema_version") {
            continue;
        }
        for (const json& wrong : {json("text"), json(-5), json(nullptr), json::array(), json::object(), json(true)}) {
            if (wrong == json::array() && (key == "world_flags" || key == "ground_items")) {
                continue;  // 빈 목록은 이 두 칸에서 올바른 값이에요
            }
            if (wrong.is_string() && key == "game_version") {
                continue;  // 글자는 game_version에서 올바른 값이에요
            }
            auto doc = base;
            doc[key] = wrong;
            ErrorList errors;
            EXPECT_FALSE(save_from_json(doc, "slot_1.json", errors).has_value()) << key << " = " << wrong.dump();
        }
    }
}

TEST(SaveReader, RejectsNonObjectDocument) {
    ErrorList errors;
    EXPECT_FALSE(save_from_json(json::array({1, 2}), "slot_1.json", errors).has_value());
    EXPECT_FALSE(errors.empty());
}

TEST(SaveReader, IgnoresUnknownExtraFields) {
    // 앞으로 더할 칸이 있어도 깨지지 않아요 (SAVE_FORMAT.md 5절)
    auto doc = save_to_json(sample_save());
    doc["future_field"] = 7;
    ErrorList errors;
    EXPECT_TRUE(save_from_json(doc, "slot_1.json", errors).has_value());
}

}  // namespace
