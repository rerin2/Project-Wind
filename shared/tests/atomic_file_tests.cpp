// 이 파일은 무엇을 하나요?
// 원자적 파일 쓰기(16a)를 숙제 검사(테스트)해요: 새로 쓰기, 덮어쓰기, 남은 .tmp 덮어쓰기,
// 쓰기가 실패하면 본 파일이 그대로인지.
#include "wind/save/atomic_file.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace {

namespace fs = std::filesystem;
using wind::save::temp_path_for;
using wind::save::write_file_atomically;

// 테스트마다 빈 임시 폴더 (끝나면 지워요)
struct TempFolder {
    fs::path path;
    explicit TempFolder(const std::string& name) : path(fs::temp_directory_path() / ("wind_atomic_" + name)) {
        fs::remove_all(path);
        fs::create_directories(path);
    }
    ~TempFolder() {
        std::error_code ignored;
        fs::remove_all(path, ignored);
    }
};

std::string read_all(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

TEST(AtomicFile, WritesNewFileAndLeavesNoTemp) {
    TempFolder dir("new");
    const auto file = dir.path / "a.json";
    std::string error;
    ASSERT_TRUE(write_file_atomically(file, "hello", error)) << error;
    EXPECT_EQ(read_all(file), "hello");
    EXPECT_FALSE(fs::exists(temp_path_for(file)));
}

TEST(AtomicFile, ReplacesExistingFile) {
    TempFolder dir("replace");
    const auto file = dir.path / "a.json";
    std::string error;
    ASSERT_TRUE(write_file_atomically(file, "old", error));
    ASSERT_TRUE(write_file_atomically(file, "new content", error)) << error;
    EXPECT_EQ(read_all(file), "new content");
}

TEST(AtomicFile, OverwritesLeftoverTempFromACrash) {
    // 지난번에 쓰다가 꺼져서 반쯤 쓴 .tmp가 남아 있어도 본 파일은 그대로이고, 다음 쓰기가 덮어써요
    TempFolder dir("leftover");
    const auto file = dir.path / "a.json";
    std::string error;
    ASSERT_TRUE(write_file_atomically(file, "old", error));
    std::ofstream(temp_path_for(file), std::ios::binary) << "{\"half";
    EXPECT_EQ(read_all(file), "old");
    ASSERT_TRUE(write_file_atomically(file, "new", error)) << error;
    EXPECT_EQ(read_all(file), "new");
}

TEST(AtomicFile, FailedWriteKeepsTheOldFile) {
    // .tmp 자리에 폴더가 있으면 임시 파일을 열 수 없어요: 쓰기 실패, 본 파일은 그대로
    TempFolder dir("blocked");
    const auto file = dir.path / "a.json";
    std::string error;
    ASSERT_TRUE(write_file_atomically(file, "old", error));
    fs::create_directories(temp_path_for(file));
    EXPECT_FALSE(write_file_atomically(file, "new", error));
    EXPECT_FALSE(error.empty());
    EXPECT_EQ(read_all(file), "old");
}

TEST(AtomicFile, CopyReplacesTarget) {
    TempFolder dir("copy");
    std::string error;
    ASSERT_TRUE(write_file_atomically(dir.path / "a", "first", error));
    ASSERT_TRUE(write_file_atomically(dir.path / "b", "second", error));
    ASSERT_TRUE(wind::save::copy_file_atomically(dir.path / "a", dir.path / "b", error)) << error;
    EXPECT_EQ(read_all(dir.path / "b"), "first");
}

}  // namespace
