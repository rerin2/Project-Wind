// 이 파일은 무엇을 하나요?
// 저장 슬롯 파일 읽기·쓰기·백업의 실제 순서예요 (save_slots.hpp, docs/SAVE_FORMAT.md 2·3절).

#include "wind/save/save_slots.hpp"

#include "wind/content/json_file.hpp"
#include "wind/save/atomic_file.hpp"
#include "wind/save/save_migration.hpp"
#include "wind/save/save_reader.hpp"
#include "wind/save/save_writer.hpp"

#include <system_error>
#include <utility>

namespace wind::save {
namespace {

constexpr int kJsonIndent = 2;  // 사람이 열어 볼 수 있게 들여쓰기 (파일 크기는 수십 KB 수준)

// 파일 이름 뒤에 글자를 붙인 위치 (slot_1.json → slot_1.json.bak)
std::filesystem::path with_suffix(std::filesystem::path path, const char* suffix) {
    path += suffix;
    return path;
}

// 오류 공책 내용을 SlotRead로 옮겨요
SlotRead failed(SaveStatus status, content::ErrorList& errors) {
    SlotRead read;
    read.status = status;
    read.errors = errors.take();
    return read;
}

}  // namespace

SaveSlots::SaveSlots(std::filesystem::path folder) : folder_(std::move(folder)) {}

std::optional<std::filesystem::path> SaveSlots::file_of(int slot, SaveKind kind) const {
    if (slot < 1 || slot > kSlotCount) {
        return std::nullopt;
    }
    // 이름은 숫자와 정해진 글자로만 만들어요 (밖에서 받은 글자가 섞이지 않아요)
    const auto name = "slot_" + std::to_string(slot) + (kind == SaveKind::Auto ? ".auto.json" : ".json");
    return folder_ / name;
}

std::optional<std::filesystem::path> SaveSlots::backup_of(int slot, SaveKind kind) const {
    const auto file = file_of(slot, kind);
    if (!file) {
        return std::nullopt;
    }
    return with_suffix(*file, ".bak");
}

SlotRead SaveSlots::read_file(const std::filesystem::path& path) {
    const auto display = path.filename().string();
    content::ErrorList errors;
    // 1) 없으면 Missing (새 게임). 확인조차 못 하면 손상으로 봐요
    std::error_code ec;
    const bool exists = std::filesystem::exists(path, ec);
    if (ec) {
        errors.add(display, "", "cannot check file: " + ec.message());
        return failed(SaveStatus::Corrupt, errors);
    }
    if (!exists) {
        return {};
    }
    // 2) 크기(4MB)·JSON 확인
    auto doc = content::read_json_file(path, display, errors, kMaxFileBytes);
    if (!doc) {
        return failed(SaveStatus::Corrupt, errors);
    }
    // 3) 버전 변환 (미래 버전은 TooNew)
    const auto migrated = migrate_save(*doc, display, errors);
    if (migrated != MigrateResult::Ok) {
        return failed(migrated == MigrateResult::TooNew ? SaveStatus::TooNew : SaveStatus::Corrupt, errors);
    }
    // 4) 칸 검사
    auto data = save_from_json(*doc, display, errors);
    if (!data) {
        return failed(SaveStatus::Corrupt, errors);
    }
    SlotRead read;
    read.status = SaveStatus::Ok;
    read.data = std::move(data);
    return read;
}

SlotRead SaveSlots::read(int slot, SaveKind kind) const {
    const auto file = file_of(slot, kind);
    if (!file) {
        SlotRead bad;
        bad.status = SaveStatus::Missing;  // 없는 슬롯 번호는 빈 슬롯처럼
        return bad;
    }
    auto read = read_file(*file);
    // 본 파일을 못 쓰면 백업이 쓸 만한지도 알려 줘요 (손상 확인 창에서 "백업 불러오기"를 보일지)
    if (read.status != SaveStatus::Ok) {
        read.backup = read_file(with_suffix(*file, ".bak")).status;
    }
    return read;
}

SlotRead SaveSlots::read_backup(int slot, SaveKind kind) const {
    const auto backup = backup_of(slot, kind);
    return backup ? read_file(*backup) : SlotRead{};
}

bool SaveSlots::write(int slot, SaveKind kind, const SaveData& data, std::string& error) {
    const auto file = file_of(slot, kind);
    if (!file) {
        error = "slot " + std::to_string(slot) + " is outside 1-" + std::to_string(kSlotCount);
        return false;
    }
    // 1) 다시 읽을 수 없는 내용은 쓰지 않아요 (게임 쪽 버그가 저장을 망치지 않게)
    const auto doc = save_to_json(data);
    content::ErrorList problems;
    if (!save_from_json(doc, file->filename().string(), problems)) {
        error = "refusing to write an unreadable save";
        if (!problems.empty()) {
            error += ": " + content::to_string(problems.items().front());
        }
        return false;
    }
    // 2) 저장 폴더 만들기
    std::error_code ec;
    std::filesystem::create_directories(folder_, ec);
    if (ec) {
        error = "cannot create save folder " + folder_.string() + ": " + ec.message();
        return false;
    }
    // 3) 미래 버전 파일은 덮어쓰지 않아요 (새 게임으로 저장한 것을 옛 게임이 지우지 않게)
    const auto current = read_file(*file).status;
    if (current == SaveStatus::TooNew) {
        error = file->filename().string() + " was written by a newer game version";
        return false;
    }
    // 4) 지금 파일이 올바를 때만 .bak으로 (손상본이 좋은 백업을 덮지 않게)
    if (current == SaveStatus::Ok && !copy_file_atomically(*file, with_suffix(*file, ".bak"), error)) {
        return false;
    }
    // 5) 새 내용을 원자적으로 덮어쓰기
    return write_file_atomically(*file, doc.dump(kJsonIndent), error);
}

bool SaveSlots::restore_backup(int slot, SaveKind kind, std::string& error) {
    const auto file = file_of(slot, kind);
    if (!file) {
        error = "slot " + std::to_string(slot) + " is outside 1-" + std::to_string(kSlotCount);
        return false;
    }
    const auto backup = with_suffix(*file, ".bak");
    if (read_file(backup).status != SaveStatus::Ok) {
        error = "backup " + backup.filename().string() + " is missing or unreadable";
        return false;
    }
    // 손상된 본 파일은 지우지 않고 .corrupt로 치워 둬요 (원인 확인용)
    std::error_code ec;
    if (std::filesystem::exists(*file, ec) && read_file(*file).status != SaveStatus::Ok && !set_aside(slot, kind, error)) {
        return false;
    }
    return copy_file_atomically(backup, *file, error);
}

bool SaveSlots::set_aside(int slot, SaveKind kind, std::string& error) {
    const auto file = file_of(slot, kind);
    if (!file) {
        error = "slot " + std::to_string(slot) + " is outside 1-" + std::to_string(kSlotCount);
        return false;
    }
    std::error_code ec;
    std::filesystem::rename(*file, with_suffix(*file, ".corrupt"), ec);  // 이전 .corrupt는 덮어써요
    if (ec) {
        error = "cannot set aside " + file->filename().string() + ": " + ec.message();
        return false;
    }
    return true;
}

}  // namespace wind::save
