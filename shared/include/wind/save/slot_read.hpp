#pragma once

// 이 파일은 무엇을 하나요?
// 저장 파일 하나를 읽은 결과예요. (docs/SAVE_FORMAT.md 3절, 16a)
// SaveSlots가 돌려주고, 16b에서 게임이 이것을 보고 불러오기·새 게임·손상 확인 창을 정해요.

#include "wind/content/error_list.hpp"
#include "wind/save/save_data.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace wind::save {

// 어느 파일인가: 본 저장(쿠션·쓰러짐) 또는 자동 저장(쓰러짐·지역 전환). 슬롯마다 하나씩이에요
enum class SaveKind : std::uint8_t { Main, Auto };

enum class SaveStatus : std::uint8_t {
    Ok,       // 읽었어요
    Missing,  // 파일이 없어요 (새 게임)
    Corrupt,  // JSON이 아니거나, 칸이 틀렸거나, 너무 커요 (백업 되살리기를 제안해요)
    TooNew,   // 모르는 미래 버전이에요 (읽지도 덮어쓰지도 않아요)
};

struct SlotRead {
    SaveStatus status = SaveStatus::Missing;
    std::optional<SaveData> data;                  // Ok일 때만 있어요
    std::vector<content::ContentError> errors;     // Corrupt·TooNew일 때 이유 (로그용)
    // 본 파일이 Ok가 아닐 때 .bak을 읽어 본 결과 (본 파일이 Ok면 확인하지 않아 Missing)
    SaveStatus backup = SaveStatus::Missing;
};

}  // namespace wind::save
