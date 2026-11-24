// 이 파일은 무엇을 하나요?
// 저장 파일 버전 변환의 실제 동작과 진짜 변환 단계 표예요 (save_migration.hpp 설명 참고).
// 새 버전을 만들 때: 아래 표 끝에 "옛 버전 → 새 버전" 함수를 하나 더하고, save_data.hpp의
// kSaveSchemaVersion을 1 올리고, docs/SAVE_FORMAT.md와 단계 테스트를 함께 고쳐요.

#include "wind/save/save_migration.hpp"

#include "wind/save/save_data.hpp"

#include <array>

namespace wind::save {
namespace {

// 진짜 단계 표 (버전 1에는 아직 없어요). 예: [0] = 1 → 2
const std::array<MigrationStep, 0> kSteps{};

// 지금 버전 = 1 + 단계 수 (표와 상수가 어긋나면 빌드가 멈춰요)
static_assert(kSaveSchemaVersion == 1 + static_cast<std::int64_t>(std::tuple_size_v<decltype(kSteps)>),
              "kSaveSchemaVersion must equal 1 + number of migration steps");

}  // namespace

std::span<const MigrationStep> save_migration_steps() {
    return kSteps;
}

MigrateResult migrate_save(nlohmann::json& doc, const std::string& file, content::ErrorList& errors,
                           std::span<const MigrationStep> steps) {
    const auto target = 1 + static_cast<std::int64_t>(steps.size());
    // 1) 버전 칸 확인: 정수여야 하고 1 이상
    const auto* version = doc.is_object() && doc.contains("schema_version") ? &doc["schema_version"] : nullptr;
    if (version == nullptr || !version->is_number_integer()) {
        errors.add(file, "schema_version", "missing or not an integer");
        return MigrateResult::Invalid;
    }
    auto current = version->get<std::int64_t>();
    if (current < 1) {
        errors.add(file, "schema_version", "version " + std::to_string(current) + " is below 1");
        return MigrateResult::Invalid;
    }
    // 2) 미래 버전은 칸 뜻을 모르니 손대지 않아요
    if (current > target) {
        errors.add(file, "schema_version",
                   "version " + std::to_string(current) + " is newer than " + std::to_string(target));
        return MigrateResult::TooNew;
    }
    // 3) 한 단계씩 올려요. 단계마다 버전 칸을 고쳐 써서, 중간에 실패해도 어디까지 왔는지 남아요
    while (current < target) {
        const auto& step = steps[static_cast<std::size_t>(current - 1)];
        if (!step || !step(doc, errors, file)) {
            errors.add(file, "schema_version",
                       "cannot upgrade from version " + std::to_string(current) + " to " + std::to_string(current + 1));
            return MigrateResult::Invalid;
        }
        ++current;
        doc["schema_version"] = current;
    }
    return MigrateResult::Ok;
}

}  // namespace wind::save
