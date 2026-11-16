// 이 파일은 무엇을 하나요?
// 마술 파일을 한 칸씩 확인하며 읽는 곳이에요.
// 순서: 공통 칸 -> MP 칸(둘 중 하나) -> 동작 칸(셋 중 하나) -> 선택 칸 -> 서로 맞는지 검사

#include "wind/content/spell_loader.hpp"

#include "wind/content/combat_readers.hpp"
#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <utility>

namespace wind::content {
namespace {

// 칸마다 허락하는 범위. 너무 크면 데이터 실수일 가능성이 커요.
constexpr std::size_t kMaxIdLength = 64;
constexpr std::size_t kMaxNameLength = 64;  // UTF-8 바이트 (한글 한 글자 = 3바이트)
constexpr std::int64_t kMinIntelligence = 1;   // 능력치 범위 (명세 8.2)
constexpr std::int64_t kMaxIntelligence = 60;
constexpr std::int64_t kMinMpCost = 1;         // 0 MP 마술은 없어요 (공짜면 마술이 아니라 공격)
constexpr std::int64_t kMaxMpCost = 999;
constexpr double kMinMpPerSecond = 0.1;
constexpr double kMaxMpPerSecond = 200.0;
constexpr double kMaxCastSeconds = 5.0;
constexpr double kMaxPower = 10.0;
constexpr double kMinBeamSize = 1.0;           // 픽셀
constexpr double kMaxBeamLength = 640.0;       // 화면 너비
constexpr double kMaxBeamWidth = 128.0;
constexpr double kMinBeamTick = 0.02;          // 초. 너무 짧으면 한 박자에 여러 번 맞아요
constexpr double kMaxBeamTick = 1.0;
constexpr double kMinLightRadius = 1.0;        // 픽셀
constexpr double kMaxLightRadius = 512.0;

// "beam": { "length": 160, "width": 12, "tick_seconds": 0.1 }
std::optional<combat::BeamData> read_beam(JsonReader& parent, const std::string& file,
                                          ErrorList& errors) {
    const auto* object = parent.object("beam");
    if (object == nullptr) {
        return std::nullopt;
    }
    JsonReader reader(*object, file, parent.field_path("beam"), errors);
    const auto length = reader.number("length", kMinBeamSize, kMaxBeamLength);
    const auto width = reader.number("width", kMinBeamSize, kMaxBeamWidth);
    const auto tick = reader.number("tick_seconds", kMinBeamTick, kMaxBeamTick);
    if (!length || !width || !tick) {
        return std::nullopt;
    }
    return combat::BeamData{static_cast<float>(*length), static_cast<float>(*width),
                            static_cast<float>(*tick)};
}

// "light": { "radius": 36, "color": "#9cc8ff", "intensity": 0.8 }
std::optional<combat::SpellLight> read_light(JsonReader& parent, const std::string& file,
                                             ErrorList& errors) {
    const auto* object = parent.object("light");
    if (object == nullptr) {
        return std::nullopt;
    }
    JsonReader reader(*object, file, parent.field_path("light"), errors);
    const auto radius = reader.number("radius", kMinLightRadius, kMaxLightRadius);
    const auto color = reader.color("color");
    const auto intensity = reader.number("intensity", 0.0, 1.0);
    if (!radius || !color || !intensity) {
        return std::nullopt;
    }
    return combat::SpellLight{static_cast<float>(*radius), *color, static_cast<float>(*intensity)};
}

// MP 칸: "mp_cost"(한 번에) 또는 "mp_per_second"(유지하는 동안) 중 꼭 하나.
void read_mp(JsonReader& reader, combat::Spell& spell) {
    const bool has_cost = reader.has("mp_cost");
    const bool has_rate = reader.has("mp_per_second");
    if (has_cost == has_rate) {  // 둘 다 있거나 둘 다 없음
        reader.error("mp_cost", "needs exactly one of mp_cost or mp_per_second");
        return;
    }
    if (has_cost) {
        if (const auto cost = reader.integer("mp_cost", kMinMpCost, kMaxMpCost)) {
            spell.mp_cost = static_cast<int>(*cost);
        }
    } else if (const auto rate = reader.number("mp_per_second", kMinMpPerSecond, kMaxMpPerSecond)) {
        spell.mp_per_second = static_cast<float>(*rate);
    }
}

// 동작 칸: projectile / melee / beam 중 꼭 하나. 고른 동작은 spell.motion에 적어요.
void read_motion(JsonReader& reader, const std::string& file, ErrorList& errors,
                 combat::Spell& spell) {
    const int kinds = static_cast<int>(reader.has("projectile")) +
                      static_cast<int>(reader.has("melee")) + static_cast<int>(reader.has("beam"));
    if (kinds != 1) {
        reader.error("projectile", "needs exactly one of projectile, melee or beam");
        return;
    }
    if (reader.has("projectile")) {
        spell.motion = combat::SpellMotion::Projectile;
        spell.projectile = read_projectile(reader, file, errors);
    } else if (reader.has("melee")) {
        spell.motion = combat::SpellMotion::Melee;
        spell.melee = read_hitbox(reader, "melee", file, errors);
    } else {
        spell.motion = combat::SpellMotion::Beam;
        spell.beam = read_beam(reader, file, errors);
    }
}

// MP 쓰는 방식과 동작이 맞는지: 광선만 1초마다 쓰고, 광선은 꼭 1초마다 써요.
void check_mp_matches_motion(JsonReader& reader, const combat::Spell& spell) {
    const bool is_beam = spell.motion == combat::SpellMotion::Beam;
    const bool per_second = spell.mp_per_second > 0.0f;
    if (is_beam && spell.mp_cost > 0) {
        reader.error("mp_cost", "a beam spell uses mp_per_second");
    } else if (!is_beam && per_second) {
        reader.error("mp_per_second", "only a beam spell uses mp_per_second");
    }
}

// 없어도 되는 파일 위치 칸 (fx, sfx)
void read_optional_path(JsonReader& reader, std::string_view key,
                        std::optional<std::string>& out) {
    if (reader.has(key)) {
        out = reader.content_path(key);
    }
}

}  // namespace

LoadResult<combat::Spell> parse_spell(const nlohmann::json& document, const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    if (!reader.valid() || !reader.schema_version(kSpellSchemaVersion)) {
        if (errors.empty()) {
            errors.add(file, "", "expected a JSON object");
        }
        return {std::nullopt, errors.take()};
    }

    combat::Spell spell;
    auto id = reader.string("id", kMaxIdLength);
    auto name = reader.string("name", kMaxNameLength);
    const auto intelligence =
        reader.integer("intelligence_required", kMinIntelligence, kMaxIntelligence);
    const auto cast = reader.number("cast_seconds", 0.0, kMaxCastSeconds);
    const auto power = reader.number("power", 0.0, kMaxPower);
    read_mp(reader, spell);
    read_motion(reader, file, errors, spell);
    read_optional_path(reader, "fx", spell.fx);
    read_optional_path(reader, "sfx", spell.sfx);
    if (reader.has("light")) {
        spell.light = read_light(reader, file, errors);
    }
    if (errors.empty()) {  // 칸이 다 맞을 때만 서로 맞는지 봐요 (같은 잘못을 두 번 적지 않게)
        check_mp_matches_motion(reader, spell);
    }

    // 틀린 칸이 하나라도 있으면 마술 전체를 거절해요 (반쯤 맞는 마술은 안 써요)
    if (!id || !name || !intelligence || !cast || !power || !errors.empty()) {
        if (errors.empty()) {
            errors.add(file, "", "spell is incomplete");
        }
        return {std::nullopt, errors.take()};
    }
    spell.id = std::move(*id);
    spell.name = std::move(*name);
    spell.intelligence_required = static_cast<int>(*intelligence);
    spell.cast_seconds = static_cast<float>(*cast);
    spell.power = static_cast<float>(*power);
    return {std::move(spell), {}};
}

LoadResult<combat::Spell> load_spell_file(const std::filesystem::path& content_root,
                                          std::string_view path) {
    ErrorList errors;
    const std::string name(path);
    const auto file = resolve_content_path(content_root, path);  // 콘텐츠 폴더 밖이면 거절
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = read_json_file(*file, name, errors);
    if (!document) {
        return {std::nullopt, errors.take()};
    }
    return parse_spell(*document, name);
}

}  // namespace wind::content
