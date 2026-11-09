// 이 파일은 무엇을 하나요?
// 맵 분위기(ambience) 묶음을 한 칸씩 확인하며 읽는 곳이에요.
//   "ambience": {
//     "ambient_fx": ["fx/petals.json"],                          (없어도 돼요)
//     "ambient_light": "#202038",                                 (없으면 조명 없음)
//     "player_light": { "radius": 72, "color": "#ffe8c0", "intensity": 0.7 },   (없어도 돼요)
//     "lights": [ { "x": 5.5, "y": 3.2, "radius": 96, "color": "#ffb060",
//                   "intensity": 1, "flicker": 0.35,
//                   "pulse": { "frequency": 0.5, "depth": 0.3 }, "fx": "fx/torch_embers.json" } ]
//   }

#include "wind/content/ambience_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/hex_color.hpp"
#include "wind/content/json_reader.hpp"

#include <optional>
#include <string_view>
#include <utility>

namespace wind::content {
namespace {

constexpr double kMinLightRadius = 4.0;    // 픽셀
constexpr double kMaxLightRadius = 512.0;
constexpr double kMaxPulseHz = 10.0;

// 색 칸을 읽어요. "#rrggbb" 모양이 아니면 오류로 적어요.
std::optional<world::Rgba> read_color(JsonReader& reader, std::string_view key) {
    const auto text = reader.string(key, 16);
    if (!text) {
        return std::nullopt;
    }
    const auto color = parse_hex_color(*text);
    if (!color) {
        reader.error(reader.field_path(key), "expected \"#rrggbb\" or \"#rrggbbaa\"");
    }
    return color;
}

// 없어도 되는 숫자 칸
void optional_number(JsonReader& reader, std::string_view key, double min, double max, float& out) {
    if (reader.has(key)) {
        if (const auto value = reader.number(key, min, max)) {
            out = static_cast<float>(*value);
        }
    }
}

// 콘텐츠 폴더 안을 가리키는 파일 위치인지 확인해요.
std::optional<std::string> read_path(JsonReader& reader, std::string_view key) {
    const auto raw = reader.string(key);
    if (!raw) {
        return std::nullopt;
    }
    auto normalized = normalize_content_path(*raw);
    if (!normalized) {
        reader.error(reader.field_path(key), "path must stay inside the content folder: " + *raw);
    }
    return normalized;
}

void parse_ambient_fx(JsonReader& reader, world::Ambience& out) {
    const auto* list = reader.array("ambient_fx", kMaxAmbientFx);
    if (list == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < list->size(); ++i) {
        const auto& item = (*list)[i];
        const auto field = reader.index_path("ambient_fx", i);
        if (!item.is_string()) {
            reader.error(field, "expected a content path string");
            continue;
        }
        auto normalized = normalize_content_path(item.get<std::string>());
        if (!normalized) {
            reader.error(field, "path must stay inside the content folder");
            continue;
        }
        out.ambient_fx.push_back(std::move(*normalized));
    }
}

void parse_player_light(JsonReader& reader, const std::string& file, ErrorList& errors,
                        world::Ambience& out) {
    const auto* object = reader.object("player_light");
    if (object == nullptr) {
        return;
    }
    JsonReader light(*object, file, reader.field_path("player_light"), errors);
    world::PlayerLight result;
    const auto radius = light.number("radius", kMinLightRadius, kMaxLightRadius);
    const auto color = read_color(light, "color");
    const auto intensity = light.number("intensity", 0.0, 1.0);
    if (radius && color && intensity) {
        result.radius = static_cast<float>(*radius);
        result.color = *color;
        result.intensity = static_cast<float>(*intensity);
        out.player_light = result;
    }
}

void parse_lights(JsonReader& reader, const std::string& file, float map_width, float map_height,
                  ErrorList& errors, world::Ambience& out) {
    const auto* list = reader.array("lights", kMaxMapLights);
    if (list == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < list->size(); ++i) {
        JsonReader light(list->at(i), file, reader.index_path("lights", i), errors);
        if (!light.valid()) {
            continue;
        }
        world::LightDef def;
        // 위치는 맵 안(0 ~ 가로·세로 칸 수)이어야 해요.
        const auto x = light.number("x", 0.0, map_width);
        const auto y = light.number("y", 0.0, map_height);
        const auto radius = light.number("radius", kMinLightRadius, kMaxLightRadius);
        const auto color = read_color(light, "color");
        const auto intensity = light.number("intensity", 0.0, 1.0);
        optional_number(light, "flicker", 0.0, 1.0, def.flicker);
        if (light.has("pulse")) {
            if (const auto* pulse = light.object("pulse")) {
                JsonReader pulse_reader(*pulse, file, light.field_path("pulse"), errors);
                if (const auto f = pulse_reader.number("frequency", 0.0, kMaxPulseHz)) {
                    def.pulse_frequency = static_cast<float>(*f);
                }
                if (const auto d = pulse_reader.number("depth", 0.0, 1.0)) {
                    def.pulse_depth = static_cast<float>(*d);
                }
            }
        }
        if (light.has("fx")) {
            if (auto fx = read_path(light, "fx")) def.fx = std::move(*fx);
        }
        if (x && y && radius && color && intensity) {
            def.x = static_cast<float>(*x);
            def.y = static_cast<float>(*y);
            def.radius = static_cast<float>(*radius);
            def.color = *color;
            def.intensity = static_cast<float>(*intensity);
            out.lights.push_back(std::move(def));
        }
    }
}

}  // namespace

void parse_ambience(const nlohmann::json& object, const std::string& file, float map_width,
                    float map_height, ErrorList& errors, world::Ambience& out) {
    JsonReader reader(object, file, "ambience", errors);
    if (!reader.valid()) {
        return;
    }
    if (reader.has("ambient_fx")) {
        parse_ambient_fx(reader, out);
    }
    if (reader.has("ambient_light")) {
        if (const auto color = read_color(reader, "ambient_light")) {
            out.ambient_light = *color;
        }
    }
    if (reader.has("player_light")) {
        parse_player_light(reader, file, errors, out);
    }
    if (reader.has("lights")) {
        parse_lights(reader, file, map_width, map_height, errors, out);
    }
}

}  // namespace wind::content
