// 이 파일은 무엇을 하나요?
// 날씨 표 파일을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/weather_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <limits>
#include <set>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxStates = 16;
constexpr std::size_t kMaxFxPerState = 4;
constexpr double kMaxWeight = 100.0;
constexpr double kMinDuration = 1.0;      // 초
constexpr double kMaxDuration = 3600.0;
constexpr double kMinTransition = 0.1;
constexpr double kMaxTransition = 60.0;

// 효과 파일 목록 칸 (없어도 돼요). 모두 콘텐츠 폴더 안이어야 해요.
void read_fx_list(JsonReader& reader, std::string_view key, std::vector<std::string>& out) {
    if (!reader.has(key)) {
        return;
    }
    const auto* list = reader.array(key, kMaxFxPerState);
    if (list == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < list->size(); ++i) {
        const auto& item = (*list)[i];
        auto normalized = item.is_string() ? normalize_content_path(item.get<std::string>())
                                           : std::nullopt;
        if (!normalized) {
            reader.error(reader.index_path(key, i), "expected a path inside the content folder");
            continue;
        }
        out.push_back(std::move(*normalized));
    }
}

}  // namespace

LoadResult<world::WeatherTable> parse_weather_table(const nlohmann::json& document,
                                                    const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    world::WeatherTable table;
    if (!reader.valid()) {
        return {std::nullopt, errors.take()};
    }
    const auto version = reader.integer("schema_version", 1, std::numeric_limits<std::int32_t>::max());
    if (version && *version != kWeatherSchemaVersion) {
        reader.error("schema_version", "unsupported schema version " + std::to_string(*version));
    }
    if (const auto t = reader.number("transition_seconds", kMinTransition, kMaxTransition)) {
        table.transition_seconds = static_cast<float>(*t);
    }
    const auto* states = reader.array("states", kMaxStates);
    if (states != nullptr && states->empty()) {
        reader.error("states", "needs at least one weather state");
    }
    std::set<std::string> ids;
    bool any_weight = false;
    for (std::size_t i = 0; states != nullptr && i < states->size(); ++i) {
        JsonReader state_reader((*states)[i], file, reader.index_path("states", i), errors);
        if (!state_reader.valid()) {
            continue;
        }
        world::WeatherState state;
        auto id = state_reader.string("id", 32);
        const auto weight = state_reader.number("weight", 0.0, kMaxWeight);
        const auto duration = state_reader.range("duration", kMinDuration, kMaxDuration);
        read_fx_list(state_reader, "fx", state.fx);
        read_fx_list(state_reader, "ground_fx", state.ground_fx);
        if (state_reader.has("tint")) {
            if (const auto tint = state_reader.color("tint")) state.tint = *tint;
        }
        if (state_reader.has("fog")) {
            if (const auto fog = state_reader.number("fog", 0.0, 1.0)) {
                state.fog = static_cast<float>(*fog);
            }
        }
        if (!id || !weight || !duration) {
            continue;
        }
        if (!ids.insert(*id).second) {
            state_reader.error(state_reader.field_path("id"), "duplicate weather id: " + *id);
            continue;
        }
        any_weight = any_weight || *weight > 0.0;
        state.id = std::move(*id);
        state.weight = static_cast<float>(*weight);
        state.min_seconds = static_cast<float>(duration->min);
        state.max_seconds = static_cast<float>(duration->max);
        table.states.push_back(std::move(state));
    }
    // 모든 날씨의 확률이 0이면 아무것도 고를 수 없어요.
    if (errors.empty() && !any_weight) {
        reader.error("states", "at least one state needs a weight above 0");
    }
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(table), {}};
}

LoadResult<world::WeatherTable> load_weather_file(const std::filesystem::path& content_root,
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
    return parse_weather_table(*document, name);
}

}  // namespace wind::content
