// 이 파일은 무엇을 하나요?
// 저장 슬롯 파일(16a)을 숙제 검사(테스트)해요. 16단계 완료 기준 두 개가 여기 있어요:
// - "저장 도중 강제 종료해도 이전 저장이 남음" (CrashMidSave..., FailedWrite...)
// - "손상된 파일에서 크래시하지 않고 백업 복구를 제안함" (CorruptFileOffersBackup..., 손상 모양 여러 개)
#include "wind/save/save_slots.hpp"

#include "wind/save/atomic_file.hpp"
#include "wind/save/save_writer.hpp"

#include "sample_save.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

namespace fs = std::filesystem;
using wind::save::SaveData;
using wind::save::SaveKind;
using wind::save::SaveSlots;
using wind::save::SaveStatus;
using wind::save::test::sample_save;

// 테스트마다 빈 저장 폴더 (끝나면 지워요). 폴더 안의 "saves"는 아직 없어요(write가 만들어요)
struct TempSaves {
    fs::path root;
    SaveSlots slots;
    explicit TempSaves(const std::string& name)
        : root(fs::temp_directory_path() / ("wind_slots_" + name)), slots(root / "saves") {
        fs::remove_all(root);
    }
    ~TempSaves() {
        std::error_code ignored;
        fs::remove_all(root, ignored);
    }
    [[nodiscard]] fs::path main_file() const { return *slots.file_of(1, SaveKind::Main); }
    void write(const SaveData& data) {
        std::string error;
        ASSERT_TRUE(slots.write(1, SaveKind::Main, data, error)) << error;
    }
};

// 같은 모양에 룬만 다른 저장 (어느 저장이 남았는지 구분용)
SaveData with_runes(std::int64_t runes) {
    auto data = sample_save();
    data.runes = runes;
    return data;
}

void overwrite(const fs::path& path, const std::string& bytes) {
    std::ofstream(path, std::ios::binary | std::ios::trunc) << bytes;
}

TEST(SaveSlots, FileNamesComeFromSlotNumbersOnly) {
    const SaveSlots slots("saves");
    EXPECT_EQ(slots.file_of(1, SaveKind::Main)->filename(), "slot_1.json");
    EXPECT_EQ(slots.file_of(3, SaveKind::Auto)->filename(), "slot_3.auto.json");
    EXPECT_EQ(slots.backup_of(2, SaveKind::Main)->filename(), "slot_2.json.bak");
    EXPECT_FALSE(slots.file_of(0, SaveKind::Main).has_value());
    EXPECT_FALSE(slots.file_of(4, SaveKind::Main).has_value());
}

TEST(SaveSlots, WriteThenReadGivesTheSameSave) {
    TempSaves saves("roundtrip");
    saves.write(sample_save());
    const auto read = saves.slots.read(1, SaveKind::Main);
    ASSERT_EQ(read.status, SaveStatus::Ok);
    EXPECT_EQ(*read.data, sample_save());
}

TEST(SaveSlots, MainAndAutoAreSeparateFiles) {
    TempSaves saves("kinds");
    std::string error;
    ASSERT_TRUE(saves.slots.write(1, SaveKind::Main, with_runes(10), error));
    ASSERT_TRUE(saves.slots.write(1, SaveKind::Auto, with_runes(20), error));
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 10);
    EXPECT_EQ(saves.slots.read(1, SaveKind::Auto).data->runes, 20);
    EXPECT_EQ(saves.slots.read(2, SaveKind::Main).status, SaveStatus::Missing);
}

TEST(SaveSlots, MissingFileIsMissing) {
    TempSaves saves("missing");
    const auto read = saves.slots.read(1, SaveKind::Main);
    EXPECT_EQ(read.status, SaveStatus::Missing);
    EXPECT_EQ(read.backup, SaveStatus::Missing);
    EXPECT_FALSE(read.data.has_value());
}

TEST(SaveSlots, BadSlotNumberDoesNotWrite) {
    TempSaves saves("badslot");
    std::string error;
    EXPECT_FALSE(saves.slots.write(4, SaveKind::Main, sample_save(), error));
    EXPECT_FALSE(error.empty());
}

TEST(SaveSlots, SecondWriteKeepsThePreviousSaveAsBackup) {
    TempSaves saves("backup");
    saves.write(with_runes(10));
    saves.write(with_runes(20));
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 20);
    // .bak을 본 파일 자리에 복사해 읽어 보면 이전 저장이에요
    const auto copy = saves.root / "check";
    fs::create_directories(copy);
    fs::copy_file(*saves.slots.backup_of(1, SaveKind::Main), copy / "slot_1.json");
    EXPECT_EQ(SaveSlots(copy).read(1, SaveKind::Main).data->runes, 10);
}

TEST(SaveSlots, CrashMidSaveKeepsThePreviousSave) {
    // 저장 도중 강제 종료: 새 내용은 .tmp에 반쯤만 쓰였고 이름 바꾸기는 일어나지 않았어요
    TempSaves saves("crash");
    saves.write(with_runes(10));
    overwrite(wind::save::temp_path_for(saves.main_file()), R"({"schema_version": 1, "hero": {"run)");
    const auto read = saves.slots.read(1, SaveKind::Main);
    ASSERT_EQ(read.status, SaveStatus::Ok);
    EXPECT_EQ(read.data->runes, 10);
    // 다음 저장은 남은 .tmp를 덮어쓰고 잘 돼요
    saves.write(with_runes(30));
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 30);
}

TEST(SaveSlots, FailedWriteKeepsThePreviousSave) {
    TempSaves saves("failed");
    saves.write(with_runes(10));
    fs::create_directories(wind::save::temp_path_for(saves.main_file()));  // .tmp를 열 수 없게
    std::string error;
    EXPECT_FALSE(saves.slots.write(1, SaveKind::Main, with_runes(20), error));
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 10);
}

TEST(SaveSlots, CorruptFileOffersBackupAndRestoresIt) {
    TempSaves saves("corrupt");
    saves.write(with_runes(10));
    saves.write(with_runes(20));  // .bak = 10
    overwrite(saves.main_file(), "\x01\x02 not json at all");
    const auto read = saves.slots.read(1, SaveKind::Main);
    EXPECT_EQ(read.status, SaveStatus::Corrupt);
    EXPECT_FALSE(read.errors.empty());
    EXPECT_EQ(read.backup, SaveStatus::Ok);  // 백업 불러오기를 제안할 수 있어요

    std::string error;
    ASSERT_TRUE(saves.slots.restore_backup(1, SaveKind::Main, error)) << error;
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 10);
    // 손상본은 지우지 않고 .corrupt로 보관해요
    auto corrupt = saves.main_file();
    corrupt += ".corrupt";
    EXPECT_TRUE(fs::exists(corrupt));
}

TEST(SaveSlots, SavingOverACorruptFileKeepsTheGoodBackup) {
    TempSaves saves("protect");
    saves.write(with_runes(10));
    saves.write(with_runes(20));  // .bak = 10
    overwrite(saves.main_file(), "{}");
    saves.write(with_runes(30));  // 손상본은 .bak으로 가지 않아요
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 30);
    overwrite(saves.main_file(), "broken");
    std::string error;
    ASSERT_TRUE(saves.slots.restore_backup(1, SaveKind::Main, error)) << error;
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).data->runes, 10);
}

TEST(SaveSlots, CorruptWithoutBackupSaysSo) {
    TempSaves saves("nobackup");
    saves.write(with_runes(10));
    overwrite(saves.main_file(), "");
    const auto read = saves.slots.read(1, SaveKind::Main);
    EXPECT_EQ(read.status, SaveStatus::Corrupt);
    EXPECT_EQ(read.backup, SaveStatus::Missing);
    std::string error;
    EXPECT_FALSE(saves.slots.restore_backup(1, SaveKind::Main, error));
}

TEST(SaveSlots, ManyKindsOfDamageAreCorruptNotCrashes) {
    TempSaves saves("damage");
    saves.write(sample_save());
    const auto good = wind::save::save_to_json(sample_save()).dump();
    for (const std::string& bad : {std::string(), good.substr(0, good.size() / 2), std::string("null"),
                                   std::string("[1,2,3]"), std::string(R"({"schema_version": 1})"),
                                   std::string(1000, '\0')}) {
        overwrite(saves.main_file(), bad);
        EXPECT_EQ(saves.slots.read(1, SaveKind::Main).status, SaveStatus::Corrupt) << bad.size();
    }
}

TEST(SaveSlots, TooLargeFileIsCorrupt) {
    TempSaves saves("large");
    saves.write(sample_save());
    overwrite(saves.main_file(), std::string(SaveSlots::kMaxFileBytes + 1, ' '));
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).status, SaveStatus::Corrupt);
}

TEST(SaveSlots, FutureVersionIsNeitherReadNorOverwritten) {
    TempSaves saves("future");
    saves.write(sample_save());
    auto doc = wind::save::save_to_json(sample_save());
    doc["schema_version"] = wind::save::kSaveSchemaVersion + 1;
    overwrite(saves.main_file(), doc.dump());
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).status, SaveStatus::TooNew);
    std::string error;
    EXPECT_FALSE(saves.slots.write(1, SaveKind::Main, sample_save(), error));
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).status, SaveStatus::TooNew);
}

TEST(SaveSlots, RefusesToWriteASaveItCouldNotReadBack) {
    TempSaves saves("unreadable");
    auto data = sample_save();
    data.position.map.clear();  // 빈 맵은 틀린 값이에요
    std::string error;
    EXPECT_FALSE(saves.slots.write(1, SaveKind::Main, data, error));
    EXPECT_FALSE(fs::exists(saves.main_file()));
}

TEST(SaveSlots, SetAsideMovesTheFile) {
    TempSaves saves("aside");
    std::string error;
    EXPECT_FALSE(saves.slots.set_aside(1, SaveKind::Main, error));  // 파일이 없어요
    saves.write(sample_save());
    ASSERT_TRUE(saves.slots.set_aside(1, SaveKind::Main, error)) << error;
    EXPECT_EQ(saves.slots.read(1, SaveKind::Main).status, SaveStatus::Missing);
}

}  // namespace
