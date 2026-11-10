// 이 파일은 무엇을 하나요?
// 파티클 효과 요리법을 한 칸씩 확인하는 "숙제 검사"를 실제로 하는 곳이에요.
// 꼭 있어야 하는 칸: schema_version, id, shape, blend, lifetime, speed, start_size,
//                    start_color, end_color, 그리고 burst나 rate 중 하나 이상
// 없어도 되는 칸: direction, spread, gravity, drag, spawn_radius, end_size, fade_in, fade_out,
//                 wobble

#include "engine/fx/particle_effect_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <cmath>
#include <cstddef>
#include <limits>

namespace wind::engine {
namespace {

using content::JsonReader;

// 받아 줄 수 있는 값의 한계. 이보다 크면 실수라고 보고 거절해요.
constexpr double kMaxLifetime = 60.0;    // 초
constexpr double kMinLifetime = 0.01;
constexpr double kMaxSpeed = 2000.0;     // 픽셀/초
constexpr double kMaxForce = 2000.0;     // 중력 크기 (픽셀/초²)
constexpr double kMaxDrag = 50.0;
constexpr double kMaxSize = 64.0;        // 픽셀
constexpr double kMaxSpawnRadius = 128.0;
constexpr double kMaxWobble = 64.0;      // 픽셀
constexpr double kMaxWobbleHz = 20.0;
constexpr std::int64_t kMaxBurst = 256;
constexpr double kMaxRate = 2000.0;      // 1초에 최대 몇 개 (센 비는 500개가 넘어요)

// 없어도 되는 숫자 칸: 있으면 검사해서 읽고, 없으면 기본값을 그대로 둬요.
void optional_number(JsonReader& reader, std::string_view key, double min, double max, float& out) {
    if (reader.has(key)) {
        if (const auto value = reader.number(key, min, max)) {
            out = static_cast<float>(*value);
        }
    }
}

// 범위 칸(숫자 하나 또는 [최소, 최대])을 효과용 범위로 읽어요. 검사는 JsonReader가 해요.
std::optional<FloatRange> read_range(JsonReader& reader, std::string_view key, double min,
                                     double max) {
    const auto range = reader.range(key, min, max);
    if (!range) {
        return std::nullopt;
    }
    return FloatRange{static_cast<float>(range->min), static_cast<float>(range->max)};
}

// 색 칸을 그리기용 색으로 읽어요. 검사는 JsonReader가 해요.
std::optional<Color> read_color(JsonReader& reader, std::string_view key) {
    const auto color = reader.color(key);
    if (!color) {
        return std::nullopt;
    }
    return Color{color->r, color->g, color->b, color->a};
}

}  // namespace

content::LoadResult<ParticleEffect> parse_particle_effect(const nlohmann::json& document,
                                                          const std::string& file) {
    content::ErrorList errors;
    JsonReader reader(document, file, "", errors);
    ParticleEffect effect;
    if (!reader.valid()) {
        return {std::nullopt, errors.take()};
    }

    const auto version =
        reader.integer("schema_version", 1, std::numeric_limits<std::int32_t>::max());
    if (version && *version != kParticleEffectSchemaVersion) {
        reader.error("schema_version", "unsupported schema version " + std::to_string(*version));
    }
    if (auto id = reader.string("id", 64)) effect.id = std::move(*id);

    if (const auto shape = reader.string("shape", 16)) {
        if (*shape == "soft") effect.shape = ParticleShape::Soft;
        else if (*shape == "pixel") effect.shape = ParticleShape::Pixel;
        else if (*shape == "streak") effect.shape = ParticleShape::Streak;
        else reader.error("shape", "must be \"soft\", \"pixel\" or \"streak\"");
    }
    if (const auto blend = reader.string("blend", 16)) {
        if (*blend == "normal") effect.blend = BlendMode::Normal;
        else if (*blend == "add") effect.blend = BlendMode::Add;
        else if (*blend == "multiply") effect.blend = BlendMode::Multiply;
        else reader.error("blend", "must be \"normal\", \"add\" or \"multiply\"");
    }

    if (auto r = read_range(reader, "lifetime", kMinLifetime, kMaxLifetime)) effect.lifetime = *r;
    if (auto r = read_range(reader, "speed", 0.0, kMaxSpeed)) effect.speed = *r;
    if (auto r = read_range(reader, "start_size", 0.0, kMaxSize)) effect.start_size = *r;
    if (auto c = read_color(reader, "start_color")) effect.start_color = *c;
    if (auto c = read_color(reader, "end_color")) effect.end_color = *c;

    optional_number(reader, "direction", -360.0, 360.0, effect.direction);
    optional_number(reader, "spread", 0.0, 360.0, effect.spread);
    optional_number(reader, "gravity", -kMaxForce, kMaxForce, effect.gravity);
    optional_number(reader, "drag", 0.0, kMaxDrag, effect.drag);
    optional_number(reader, "spawn_radius", 0.0, kMaxSpawnRadius, effect.spawn_radius);
    optional_number(reader, "end_size", 0.0, kMaxSize, effect.end_size);
    optional_number(reader, "fade_in", 0.0, 1.0, effect.fade_in);
    optional_number(reader, "fade_out", 0.0, 1.0, effect.fade_out);
    optional_number(reader, "rate", 0.0, kMaxRate, effect.rate);
    if (reader.has("burst")) {
        if (const auto burst = reader.integer("burst", 0, kMaxBurst)) {
            effect.burst = static_cast<std::uint32_t>(*burst);
        }
    }
    if (reader.has("wobble")) {
        if (const auto* wobble = reader.object("wobble")) {
            JsonReader wobble_reader(*wobble, file, "wobble", errors);
            if (const auto a = wobble_reader.number("amplitude", 0.0, kMaxWobble)) {
                effect.wobble_amplitude = static_cast<float>(*a);
            }
            if (const auto f = wobble_reader.number("frequency", 0.0, kMaxWobbleHz)) {
                effect.wobble_frequency = static_cast<float>(*f);
            }
        }
    }

    // 한 번에 터뜨리지도, 계속 뿜지도 않는 효과는 아무것도 안 만드니 실수예요.
    if (errors.empty() && effect.burst == 0 && effect.rate <= 0.0f) {
        reader.error("burst", "an effect needs \"burst\" > 0 or \"rate\" > 0");
    }

    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(effect), {}};
}

content::LoadResult<ParticleEffect> load_particle_effect_file(
    const std::filesystem::path& content_root, std::string_view path) {
    content::ErrorList errors;
    const std::string name(path);
    // 1) 파일 위치가 콘텐츠 폴더 안인지 확인
    const auto file = content::resolve_content_path(content_root, path);
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    // 2) 읽어서 JSON으로 바꾸고  3) 검사
    const auto document = content::read_json_file(*file, name, errors);
    if (!document) {
        return {std::nullopt, errors.take()};
    }
    return parse_particle_effect(*document, name);
}

}  // namespace wind::engine
