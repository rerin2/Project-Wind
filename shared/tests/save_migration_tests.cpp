// 이 파일은 무엇을 하나요?
// 저장 파일 버전 변환 틀(16a)을 숙제 검사(테스트)해요.
// 진짜 단계는 버전 2부터 생기므로, 가짜 단계 두 개(1→2 칸 이름 바꾸기, 2→3 칸 더하기)로 틀을 시험해요.
#include "wind/save/save_migration.hpp"

#include "wind/save/save_data.hpp"

#include <gtest/gtest.h>

#include <array>

namespace {

using nlohmann::json;
using wind::content::ErrorList;
using wind::save::MigrateResult;
using wind::save::MigrationStep;
using wind::save::migrate_save;

// 가짜 단계 표: [0] 1→2 "gold"를 "runes"로, [1] 2→3 "world_flags" 빈 목록 더하기
const std::array<MigrationStep, 2> kFakeSteps{
    [](json& doc, ErrorList& errors, const std::string& file) {
        if (!doc.contains("gold")) {
            errors.add(file, "gold", "missing");
            return false;
        }
        doc["runes"] = doc["gold"];
        doc.erase("gold");
        return true;
    },
    [](json& doc, ErrorList&, const std::string&) {
        doc["world_flags"] = json::array();
        return true;
    },
};

TEST(SaveMigration, CurrentVersionHasNoStepsYet) {
    EXPECT_EQ(wind::save::kSaveSchemaVersion, 1);
    EXPECT_TRUE(wind::save::save_migration_steps().empty());
}

TEST(SaveMigration, CurrentVersionPassesUnchanged) {
    json doc = {{"schema_version", 1}, {"runes", 5}};
    const auto before = doc;
    ErrorList errors;
    EXPECT_EQ(migrate_save(doc, "slot_1.json", errors), MigrateResult::Ok);
    EXPECT_EQ(doc, before);
}

TEST(SaveMigration, RunsEveryStepInOrder) {
    json doc = {{"schema_version", 1}, {"gold", 7}};
    ErrorList errors;
    EXPECT_EQ(migrate_save(doc, "slot_1.json", errors, kFakeSteps), MigrateResult::Ok);
    EXPECT_EQ(doc["schema_version"], 3);
    EXPECT_EQ(doc["runes"], 7);
    EXPECT_FALSE(doc.contains("gold"));
    EXPECT_TRUE(doc["world_flags"].is_array());
}

TEST(SaveMigration, StartsFromTheFilesVersion) {
    // 버전 2 파일은 두 번째 단계만 돌아요 (첫 단계를 돌면 "gold"가 없어 실패했을 거예요)
    json doc = {{"schema_version", 2}, {"runes", 7}};
    ErrorList errors;
    EXPECT_EQ(migrate_save(doc, "slot_1.json", errors, kFakeSteps), MigrateResult::Ok);
    EXPECT_EQ(doc["schema_version"], 3);
}

TEST(SaveMigration, FutureVersionIsTooNew) {
    json doc = {{"schema_version", 4}};
    const auto before = doc;
    ErrorList errors;
    EXPECT_EQ(migrate_save(doc, "slot_1.json", errors, kFakeSteps), MigrateResult::TooNew);
    EXPECT_EQ(doc, before);  // 손대지 않아요
}

TEST(SaveMigration, FailedStepIsInvalid) {
    json doc = {{"schema_version", 1}};  // "gold"가 없어 첫 단계가 실패해요
    ErrorList errors;
    EXPECT_EQ(migrate_save(doc, "slot_1.json", errors, kFakeSteps), MigrateResult::Invalid);
    EXPECT_FALSE(errors.empty());
}

TEST(SaveMigration, MissingOrBadVersionIsInvalid) {
    ErrorList errors;
    for (json doc : {json{{"runes", 1}}, json{{"schema_version", "1"}}, json{{"schema_version", 0}},
                     json{{"schema_version", 1.5}}, json::array(), json(nullptr)}) {
        EXPECT_EQ(migrate_save(doc, "slot_1.json", errors, kFakeSteps), MigrateResult::Invalid) << doc.dump();
    }
}

}  // namespace
