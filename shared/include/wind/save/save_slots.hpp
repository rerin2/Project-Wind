#pragma once

// 이 파일은 무엇을 하나요?
// 저장 폴더 안의 슬롯 파일들을 읽고 쓰는 관리자예요. (명세 12.1·12.2, docs/SAVE_FORMAT.md 1~3절, 16a)
// - 파일 이름은 슬롯 번호(1~3)와 종류(본/자동)로만 만들어요. 그래서 저장 폴더 밖으로 나갈 수 없어요.
// - 쓰기: 지금 파일이 올바르면 .bak으로 복사 → 새 내용을 원자적으로 덮어쓰기.
//   손상된 파일은 .bak으로 옮기지 않아 좋은 백업을 지켜요. 다시 읽을 수 없는 내용은 쓰지 않아요.
// - 읽기: 크기(4MB) → JSON → 버전 변환 → 칸 검사. 어디서 실패해도 크래시하지 않고 결과로 알려요.
// - 손상됐을 때: 백업 되살리기(restore_backup), 손상본을 옆으로 치우기(set_aside).
// 화면·SDL을 몰라요. 저장 폴더 위치는 앱이 정해서 넘겨요(%APPDATA%\ProjectWIND\saves).

#include "wind/save/slot_read.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace wind::save {

struct SaveData;

class SaveSlots {
public:
    static constexpr int kSlotCount = 3;                              // 슬롯 1~3 (명세 12.1)
    static constexpr std::uintmax_t kMaxFileBytes = 4u * 1024u * 1024u;  // 저장 파일 크기 상한 (명세 12.2)

    explicit SaveSlots(std::filesystem::path folder);

    // 슬롯 파일 위치 (slot_1.json, slot_1.auto.json). 슬롯 번호가 1~3 밖이면 nullopt
    [[nodiscard]] std::optional<std::filesystem::path> file_of(int slot, SaveKind kind) const;
    // 그 파일의 백업 위치 (<이름>.bak)
    [[nodiscard]] std::optional<std::filesystem::path> backup_of(int slot, SaveKind kind) const;

    // 저장해요. 성공하면 true. 실패하면 본 파일은 그대로이고 error에 이유를 적어요.
    bool write(int slot, SaveKind kind, const SaveData& data, std::string& error);
    // 읽어요. 본 파일이 Ok가 아니면 백업 상태도 알려 줘요.
    [[nodiscard]] SlotRead read(int slot, SaveKind kind) const;
    // 백업을 본 파일로 되살려요(손상본은 .corrupt로 치워요). 백업이 Ok가 아니면 아무것도 안 하고 false
    bool restore_backup(int slot, SaveKind kind, std::string& error);
    // 손상된 본 파일을 <이름>.corrupt로 옮겨요(새 게임 전에, 지우지 않고 보관). 파일이 없으면 false
    bool set_aside(int slot, SaveKind kind, std::string& error);

private:
    // 파일 하나를 읽어 검사해요 (백업 확인 없이)
    [[nodiscard]] static SlotRead read_file(const std::filesystem::path& path);

    std::filesystem::path folder_;  // 저장 폴더
};

}  // namespace wind::save
