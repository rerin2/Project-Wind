// 이 파일은 무엇을 하나요?
// 허수아비 설명 파일을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/training_dummy_loader.hpp"

#include "wind/content/body_readers.hpp"
#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <optional>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxIdLength = 64;    // id 글자 수 상한 (바이트)
constexpr std::size_t kMaxNameLength = 64;  // 이름 상한 (바이트, 한글 한 글자 = 3바이트)
constexpr std::int64_t kMaxHp = 99999;      // HP 상한 (피해 상한과 같아요)
constexpr double kMaxDefense = 9999.0;      // 방어력 상한
constexpr double kMinRecover = 0.1;         // 다시 서기까지 최소 (초)
constexpr double kMaxRecover = 30.0;        // 다시 서기까지 최대 (초)

}  // namespace

LoadResult<combat::TrainingDummyData> parse_training_dummy(const nlohmann::json& document,
                                                           const std::string& file) {
    ErrorList errors;                                    // 틀린 곳 공책
    JsonReader reader(document, file, "", errors);       // 맨 바깥 { } 읽기 도우미
    if (!reader.valid() || !reader.schema_version(kTrainingDummySchemaVersion)) {
        if (errors.empty()) errors.add(file, "", "expected a JSON object");
        return {std::nullopt, errors.take()};
    }
    auto id = reader.string("id", kMaxIdLength);                                  // 이름표
    auto name = reader.string("name", kMaxNameLength);                            // 보여 줄 이름
    const auto max_hp = reader.integer("max_hp", 1, kMaxHp);                      // 가득 찬 HP
    const auto defense = reader.number("defense", 0.0, kMaxDefense);              // 방어력
    const auto recover = reader.number("recover_seconds", kMinRecover, kMaxRecover);  // 다시 서기까지
    const auto hurtbox = read_hurtbox(reader);                                    // 맞는 상자
    auto sprite = read_sprite_strip(reader);                                      // 그림
    if (!id || !name || !max_hp || !defense || !recover || !hurtbox || !sprite || !errors.empty()) {
        if (errors.empty()) errors.add(file, "", "training dummy is incomplete");
        return {std::nullopt, errors.take()};
    }
    combat::TrainingDummyData data;                       // 다 통과! 값을 담아요
    data.id = std::move(*id);
    data.name = std::move(*name);
    data.max_hp = static_cast<int>(*max_hp);
    data.defense = static_cast<float>(*defense);
    data.recover_seconds = static_cast<float>(*recover);
    data.hurtbox = *hurtbox;
    data.sprite = std::move(*sprite);
    return {std::move(data), {}};
}

LoadResult<combat::TrainingDummyData> load_training_dummy_file(
    const std::filesystem::path& content_root, std::string_view path) {
    ErrorList errors;
    const std::string name(path);
    const auto file = resolve_content_path(content_root, path);  // 콘텐츠 폴더 안인지 문지기 검사
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = read_json_file(*file, name, errors);  // 파일 읽어서 JSON으로
    if (!document) return {std::nullopt, errors.take()};
    return parse_training_dummy(*document, name);  // 칸마다 검사
}

}  // namespace wind::content
