// 이 파일은 무엇을 하나요?
// 허수아비 설명 파일을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/training_dummy_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <array>
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
constexpr double kMaxOffset = 128.0;        // 허트박스가 발에서 떨어질 수 있는 거리 (픽셀)
constexpr double kMaxBoxSize = 128.0;       // 허트박스 최대 크기 (픽셀)
constexpr std::int64_t kMaxFrame = 128;     // 그림 한 칸 최대 크기 (픽셀)

// [a, b] 두 숫자 목록 읽기. 둘 다 min~max 안의 숫자여야 해요.
std::optional<std::array<double, 2>> read_pair(JsonReader& reader, std::string_view key, double min,
                                               double max) {
    const auto* list = reader.array(key, 2);  // 칸이 없거나 목록이 아니면 오류를 적어 줘요
    if (list == nullptr) return std::nullopt;
    if (list->size() != 2) {                  // 꼭 두 개
        reader.error(reader.field_path(key), "expected [a, b]");
        return std::nullopt;
    }
    std::array<double, 2> out{};
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& value = (*list)[i];                                  // i번째 숫자
        if (!value.is_number() || value.get<double>() < min || value.get<double>() > max) {
            reader.error(reader.index_path(key, i),
                         "must be a number " + std::to_string(min) + " to " + std::to_string(max));
            return std::nullopt;
        }
        out[i] = value.get<double>();
    }
    return out;
}

// "hurtbox": { "offset": [x, y], "size": [w, h] }
std::optional<combat::HurtboxShape> read_hurtbox(JsonReader& reader) {
    const auto* object = reader.object("hurtbox");
    if (object == nullptr) return std::nullopt;
    JsonReader box(*object, reader.file(), reader.field_path("hurtbox"), reader.errors());
    const auto offset = read_pair(box, "offset", -kMaxOffset, kMaxOffset);  // 발에서 왼쪽 위 모서리까지
    const auto size = read_pair(box, "size", 1.0, kMaxBoxSize);             // 가로·세로
    if (!offset || !size) return std::nullopt;
    return combat::HurtboxShape{static_cast<float>((*offset)[0]), static_cast<float>((*offset)[1]),
                                static_cast<float>((*size)[0]), static_cast<float>((*size)[1])};
}

// "sprite": { "image": "...", "frame": [w, h], "origin": [x, y] }
std::optional<combat::DummySprite> read_sprite(JsonReader& reader) {
    const auto* object = reader.object("sprite");
    if (object == nullptr) return std::nullopt;
    JsonReader sprite(*object, reader.file(), reader.field_path("sprite"), reader.errors());
    auto image = sprite.content_path("image");                                       // 그림 파일 (폴더 안)
    const auto frame = read_pair(sprite, "frame", 1.0, static_cast<double>(kMaxFrame));  // 한 칸 크기
    const auto origin = read_pair(sprite, "origin", 0.0, static_cast<double>(kMaxFrame));  // 발 위치
    if (!image || !frame || !origin) return std::nullopt;
    if ((*origin)[0] > (*frame)[0] || (*origin)[1] > (*frame)[1]) {  // 발 위치는 칸 안이어야 해요
        sprite.error(sprite.field_path("origin"), "origin must be inside the frame");
        return std::nullopt;
    }
    return combat::DummySprite{std::move(*image), static_cast<int>((*frame)[0]),
                               static_cast<int>((*frame)[1]), static_cast<int>((*origin)[0]),
                               static_cast<int>((*origin)[1])};
}

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
    auto sprite = read_sprite(reader);                                            // 그림
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
