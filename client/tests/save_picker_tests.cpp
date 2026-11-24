// 이 파일은 무엇을 하나요?
// 시작할 때 저장 고르기(16b)를 숙제 검사(테스트)해요: 빈 슬롯은 새 게임, 본/자동 중 새것,
// 손상(모양 또는 콘텐츠)이면 질문, 백업 되살리기·손상본 치우기 뒤 다시 확인, 미래 버전은 손대지 않음.
// 콘텐츠 확인은 가짜 함수예요: 룬이 666이면 "콘텐츠와 안 맞음"으로 봐요.
#include "game/save/save_picker.hpp"

#include "wind/save/save_slots.hpp"
#include "wind/save/save_writer.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

namespace fs = std::filesystem;
using wind::game::SavePicker;
using wind::save::SaveKind;
using Start = SavePicker::Start;

constexpr std::int64_t kMismatch = 666;  // 가짜 콘텐츠 확인이 거절하는 룬

// 모양이 맞는 가장 작은 저장
wind::save::SaveData save_with(std::int64_t runes, std::int64_t saved_at) {
    wind::save::SaveData d;
    d.game_version = "test";
    d.saved_at = saved_at;
    d.runes = runes;
    d.position = {"maps/test_meadow.json", 208.0f, 408.0f};
    d.bag.resize(1);
    return d;
}

struct Slots {
    fs::path folder;
    wind::save::SaveSlots slots;
    SavePicker picker;
    explicit Slots(const std::string& name)
        : folder(fs::temp_directory_path() / ("wind_picker_" + name)), slots(folder),
          picker(slots, 1, [](const wind::save::SaveData& data, std::vector<std::string>& problems) {
              if (data.runes != kMismatch) return true;
              problems.push_back("content mismatch");
              return false;
          }) {
        fs::remove_all(folder);
    }
    ~Slots() {
        std::error_code ignored;
        fs::remove_all(folder, ignored);
    }
    void write(SaveKind kind, const wind::save::SaveData& data) {
        std::string error;
        ASSERT_TRUE(slots.write(1, kind, data, error)) << error;
    }
    void damage(SaveKind kind) { std::ofstream(*slots.file_of(1, kind), std::ios::trunc) << "{broken"; }
};

TEST(SavePicker, EmptySlotIsANewGame) {
    Slots s("empty");
    EXPECT_EQ(s.picker.examine().start, Start::NewGame);
}

TEST(SavePicker, LoadsTheNewerOfMainAndAuto) {
    Slots s("newer");
    s.write(SaveKind::Main, save_with(10, 100));
    s.write(SaveKind::Auto, save_with(20, 200));
    auto outcome = s.picker.examine();
    ASSERT_EQ(outcome.start, Start::Load);
    EXPECT_EQ(outcome.from, SaveKind::Auto);
    EXPECT_EQ(outcome.data->runes, 20);

    s.write(SaveKind::Main, save_with(30, 300));
    EXPECT_EQ(s.picker.examine().data->runes, 30);
    s.write(SaveKind::Auto, save_with(40, 300));  // 같은 때면 본 저장
    EXPECT_EQ(s.picker.examine().from, SaveKind::Main);
}

TEST(SavePicker, CorruptFileAsksAndBackupRestores) {
    Slots s("corrupt");
    s.write(SaveKind::Main, save_with(10, 100));
    s.write(SaveKind::Main, save_with(20, 200));  // .bak = 10
    s.damage(SaveKind::Main);
    const auto outcome = s.picker.examine();
    EXPECT_EQ(outcome.start, Start::NeedsChoice);
    EXPECT_TRUE(outcome.backup_offered);
    EXPECT_FALSE(outcome.problems.empty());

    std::string error;
    ASSERT_TRUE(s.picker.use_backups(error)) << error;
    const auto after = s.picker.examine();
    ASSERT_EQ(after.start, Start::Load);
    EXPECT_EQ(after.data->runes, 10);
    auto corrupt = *s.slots.file_of(1, SaveKind::Main);
    corrupt += ".corrupt";
    EXPECT_TRUE(fs::exists(corrupt));  // 손상본은 보관
}

TEST(SavePicker, ContentMismatchCountsAsCorrupt) {
    Slots s("mismatch");
    s.write(SaveKind::Main, save_with(10, 100));
    s.write(SaveKind::Main, save_with(kMismatch, 200));  // 모양은 맞지만 콘텐츠와 안 맞음
    const auto outcome = s.picker.examine();
    EXPECT_EQ(outcome.start, Start::NeedsChoice);
    EXPECT_TRUE(outcome.backup_offered);
    std::string error;
    ASSERT_TRUE(s.picker.use_backups(error)) << error;
    EXPECT_EQ(s.picker.examine().data->runes, 10);
}

TEST(SavePicker, BackupThatAlsoMismatchesIsNotOffered) {
    Slots s("badbackup");
    s.write(SaveKind::Main, save_with(kMismatch, 100));
    s.write(SaveKind::Main, save_with(kMismatch, 200));  // .bak도 666... 하지만 손상본은 .bak이 되지 않아요
    // 위 두 번째 쓰기에서 첫 파일은 "모양이 Ok"라 .bak이 됐어요(SaveSlots는 콘텐츠를 몰라요)
    const auto outcome = s.picker.examine();
    EXPECT_EQ(outcome.start, Start::NeedsChoice);
    EXPECT_FALSE(outcome.backup_offered);
    std::string error;
    ASSERT_TRUE(s.picker.use_backups(error)) << error;  // 쓸 백업이 없으면 치우기만
    EXPECT_EQ(s.picker.examine().start, Start::NewGame);
}

TEST(SavePicker, StartFreshSetsAsideOnlyTheCorruptFile) {
    Slots s("fresh");
    s.write(SaveKind::Main, save_with(10, 100));
    s.write(SaveKind::Auto, save_with(20, 50));
    s.damage(SaveKind::Main);
    std::string error;
    ASSERT_TRUE(s.picker.set_aside_corrupt(error)) << error;
    const auto after = s.picker.examine();  // 올바른 자동 저장은 남아요
    ASSERT_EQ(after.start, Start::Load);
    EXPECT_EQ(after.from, SaveKind::Auto);
}

TEST(SavePicker, NewerVersionIsTooNewAndUntouched) {
    Slots s("toonew");
    s.write(SaveKind::Main, save_with(10, 100));
    auto doc = wind::save::save_to_json(save_with(10, 100));
    doc["schema_version"] = wind::save::kSaveSchemaVersion + 1;
    std::ofstream(*s.slots.file_of(1, SaveKind::Auto), std::ios::trunc) << doc.dump();
    EXPECT_EQ(s.picker.examine().start, Start::TooNew);
    EXPECT_TRUE(fs::exists(*s.slots.file_of(1, SaveKind::Auto)));
}

}  // namespace
