// 이 파일은 무엇을 하나요?
// 하루 색 곡선 파일을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/day_cycle_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <limits>

namespace wind::content {
namespace {

constexpr std::size_t kMinKeys = 2;
constexpr std::size_t kMaxKeys = 32;
// 24시는 0시와 같은 시각이라 받지 않아요 (24보다 아주 조금 작은 값까지).
constexpr double kLastHour = 23.999;

}  // namespace

LoadResult<world::DayCycle> parse_day_cycle(const nlohmann::json& document, const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    world::DayCycle cycle;
    if (!reader.valid()) {
        return {std::nullopt, errors.take()};
    }
    const auto version = reader.integer("schema_version", 1, std::numeric_limits<std::int32_t>::max());
    if (version && *version != kDayCycleSchemaVersion) {
        reader.error("schema_version", "unsupported schema version " + std::to_string(*version));
    }
    if (const auto* keys = reader.array("keys", kMaxKeys)) {
        if (keys->size() < kMinKeys) {
            reader.error("keys", "needs at least " + std::to_string(kMinKeys) + " keys");
        }
        double previous = -1.0;
        for (std::size_t i = 0; i < keys->size(); ++i) {
            JsonReader key_reader((*keys)[i], file, reader.index_path("keys", i), errors);
            if (!key_reader.valid()) {
                continue;
            }
            const auto hour = key_reader.number("hour", 0.0, kLastHour);
            const auto ambient = key_reader.color("ambient");
            const auto night = key_reader.number("night", 0.0, 1.0);
            if (!hour || !ambient || !night) {
                continue;
            }
            // 시각은 앞 점보다 커야 해요 (같은 시각 두 번, 거꾸로 된 순서는 실수예요).
            if (*hour <= previous) {
                key_reader.error(key_reader.field_path("hour"), "hours must increase from key to key");
                continue;
            }
            previous = *hour;
            cycle.keys.push_back({static_cast<float>(*hour), *ambient, static_cast<float>(*night)});
        }
    }
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(cycle), {}};
}

LoadResult<world::DayCycle> load_day_cycle_file(const std::filesystem::path& content_root,
                                                std::string_view path) {
    ErrorList errors;
    const std::string name(path);
    const auto file = resolve_content_path(content_root, path);
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = read_json_file(*file, name, errors);
    if (!document) {
        return {std::nullopt, errors.take()};
    }
    return parse_day_cycle(*document, name);
}

}  // namespace wind::content
