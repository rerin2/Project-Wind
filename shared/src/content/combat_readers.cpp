// 이 파일은 무엇을 하나요?
// 히트박스·날아가는 공격 칸을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/combat_readers.hpp"

#include <cmath>

namespace wind::content {
namespace {

// 칸마다 허락하는 범위. 너무 크면 데이터 실수일 가능성이 커요.
constexpr double kMaxHitboxOffset = 128.0;  // 픽셀
constexpr double kMinHitboxSize = 1.0;
constexpr double kMaxHitboxSize = 256.0;
constexpr double kMinProjectileSpeed = 1.0;      // 픽셀/초 (0이면 영원히 안 날아가요)
constexpr double kMaxProjectileSpeed = 2000.0;
constexpr double kMinProjectileRange = 1.0;      // 픽셀
constexpr double kMaxProjectileRange = 2000.0;
constexpr double kMinProjectileSize = 1.0;       // 픽셀
constexpr double kMaxProjectileSize = 64.0;
constexpr std::int64_t kMaxProjectileCount = 16;
constexpr double kMaxOrbitSeconds = 10.0;

}  // namespace

std::optional<std::array<double, 2>> read_number_pair(JsonReader& reader, std::string_view key,
                                                      double min, double max) {
    const auto* value = reader.raw(key);
    if (value == nullptr) {
        return std::nullopt;  // 없는 칸은 raw()가 이미 공책에 적었어요
    }
    if (!value->is_array() || value->size() != 2 || !(*value)[0].is_number() ||
        !(*value)[1].is_number()) {
        reader.error(reader.field_path(key), "expected [number, number]");
        return std::nullopt;
    }
    const std::array<double, 2> pair{(*value)[0].get<double>(), (*value)[1].get<double>()};
    for (const double v : pair) {
        // NaN·무한대도 여기서 걸러요
        if (!std::isfinite(v) || v < min || v > max) {
            reader.error(reader.field_path(key), "values must be " + std::to_string(min) + "-" +
                                                     std::to_string(max));
            return std::nullopt;
        }
    }
    return pair;
}

std::optional<combat::HitboxShape> read_hitbox(JsonReader& parent, std::string_view key,
                                               const std::string& file, ErrorList& errors) {
    const auto* object = parent.object(key);
    if (object == nullptr) {
        return std::nullopt;
    }
    JsonReader reader(*object, file, parent.field_path(key), errors);
    const auto offset = read_number_pair(reader, "offset", -kMaxHitboxOffset, kMaxHitboxOffset);
    const auto size = read_number_pair(reader, "size", kMinHitboxSize, kMaxHitboxSize);
    if (!offset || !size) {
        return std::nullopt;
    }
    return combat::HitboxShape{static_cast<float>((*offset)[0]), static_cast<float>((*offset)[1]),
                               static_cast<float>((*size)[0]), static_cast<float>((*size)[1])};
}

std::optional<combat::ProjectileData> read_projectile(JsonReader& parent, const std::string& file,
                                                      ErrorList& errors) {
    const auto* object = parent.object("projectile");
    if (object == nullptr) {
        return std::nullopt;
    }
    JsonReader reader(*object, file, parent.field_path("projectile"), errors);
    const auto speed = reader.number("speed", kMinProjectileSpeed, kMaxProjectileSpeed);
    const auto range = reader.number("range", kMinProjectileRange, kMaxProjectileRange);
    const auto size = read_number_pair(reader, "size", kMinProjectileSize, kMaxProjectileSize);
    combat::ProjectileData data;
    bool optional_ok = true;  // 선택 칸이 있는데 틀렸으면 false
    // 선택 칸: 있으면 범위를 확인하고, 없으면 기본값(1개, 유도 없음, 돌기 없음)
    if (reader.has("count")) {
        const auto count = reader.integer("count", 1, kMaxProjectileCount);
        optional_ok = optional_ok && count.has_value();
        if (count) data.count = static_cast<int>(*count);
    }
    if (reader.has("homing")) {
        const auto homing = reader.number("homing", 0.0, 1.0);
        optional_ok = optional_ok && homing.has_value();
        if (homing) data.homing = static_cast<float>(*homing);
    }
    if (reader.has("orbit_seconds")) {
        const auto orbit = reader.number("orbit_seconds", 0.0, kMaxOrbitSeconds);
        optional_ok = optional_ok && orbit.has_value();
        if (orbit) data.orbit_seconds = static_cast<float>(*orbit);
    }
    if (!speed || !range || !size || !optional_ok) {
        return std::nullopt;
    }
    data.speed = static_cast<float>(*speed);
    data.range = static_cast<float>(*range);
    data.width = static_cast<float>((*size)[0]);
    data.height = static_cast<float>((*size)[1]);
    return data;
}

}  // namespace wind::content
