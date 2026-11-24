#pragma once

// 이 파일은 무엇을 하나요?
// 옛 버전 저장 파일을 지금 버전 모양으로 올리는 "버전 변환"이에요. (명세 12.2, docs/SAVE_FORMAT.md 4절, 16a)
// - 변환 단계 i는 버전 i → i+1 하나만 바꿔요. 지금 버전 = 1 + 단계 수.
// - schema_version이 없거나 1보다 작으면 Invalid, 지금보다 크면(미래 게임이 쓴 파일) TooNew예요.
// - 한 단계라도 실패하면 Invalid예요. 변환은 메모리 안에서만 하고 파일은 다음 저장 때 새 버전으로 써져요.
// 테스트는 가짜 단계 목록을 넘겨 틀을 시험해요(진짜 단계는 버전 2부터 생겨요).

#include "wind/content/error_list.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <functional>
#include <span>
#include <string>

namespace wind::save {

// 한 단계: doc을 다음 버전 모양으로 바꿔요. 바꿀 수 없으면 errors에 적고 false
using MigrationStep = std::function<bool(nlohmann::json& doc, content::ErrorList& errors, const std::string& file)>;

enum class MigrateResult : std::uint8_t {
    Ok,       // 지금 버전이 됐어요 (이미 지금 버전이었어도 Ok)
    TooNew,   // 모르는 미래 버전: 읽지도 덮어쓰지도 않아요
    Invalid,  // 버전 칸이 틀렸거나 단계가 실패했어요 (손상으로 다뤄요)
};

// 진짜 변환 단계 목록 (단계 i = 버전 i+1 → i+2). 지금은 비어 있어요(버전 1)
[[nodiscard]] std::span<const MigrationStep> save_migration_steps();

// doc을 steps의 마지막 버전(1 + steps 수)까지 올리고 schema_version을 고쳐 써요.
[[nodiscard]] MigrateResult migrate_save(nlohmann::json& doc, const std::string& file, content::ErrorList& errors,
                                         std::span<const MigrationStep> steps = save_migration_steps());

}  // namespace wind::save
