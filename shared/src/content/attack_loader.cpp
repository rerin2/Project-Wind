// 이 파일은 무엇을 하나요?
// 공격 데이터 파일을 한 칸씩 확인하며 읽는 곳이에요.

#include "wind/content/attack_loader.hpp"

#include "wind/content/combat_readers.hpp"
#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <optional>
#include <utility>

namespace wind::content {
namespace {

// 칸마다 허락하는 범위. 너무 크면 데이터 실수일 가능성이 커요.
constexpr std::size_t kMaxIdLength = 64;
constexpr double kMaxWindup = 2.0;      // 초
constexpr double kMinActive = 0.01;     // 판정이 0초면 아무도 못 맞혀요
constexpr double kMaxActive = 2.0;
constexpr double kMaxRecovery = 3.0;
constexpr double kMaxPower = 10.0;
constexpr double kMaxKnockback = 1000.0;  // 픽셀/초
constexpr double kMaxHitstop = 0.5;       // 초
constexpr std::int64_t kMaxMpCost = 999;

// 없어도 되는 파일 위치 칸 (fx, sfx)
void read_optional_path(JsonReader& reader, std::string_view key,
                        std::optional<std::string>& out) {
    if (reader.has(key)) {
        out = reader.content_path(key);
    }
}

}  // namespace

LoadResult<combat::AttackData> parse_attack(const nlohmann::json& document,
                                            const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    if (!reader.valid() || !reader.schema_version(kAttackSchemaVersion)) {
        if (errors.empty()) {
            errors.add(file, "", "expected a JSON object");
        }
        return {std::nullopt, errors.take()};
    }

    combat::AttackData attack;
    auto id = reader.string("id", kMaxIdLength);
    const auto windup = reader.number("windup", 0.0, kMaxWindup);
    const auto active = reader.number("active", kMinActive, kMaxActive);
    const auto recovery = reader.number("recovery", 0.0, kMaxRecovery);
    const auto cancel = reader.range("cancel_window", 0.0, kMaxWindup + kMaxActive + kMaxRecovery);
    const auto hitbox = read_hitbox(reader, "hitbox", file, errors);
    const auto power = reader.number("power", 0.0, kMaxPower);
    const auto knockback = reader.number("knockback", 0.0, kMaxKnockback);
    const auto hitstop = reader.number("hitstop", 0.0, kMaxHitstop);
    const auto shake = reader.number("shake", 0.0, 1.0);
    if (reader.has("mp_cost")) {
        if (const auto mp = reader.integer("mp_cost", 0, kMaxMpCost)) {
            attack.mp_cost = static_cast<int>(*mp);
        }
    }
    if (reader.has("projectile")) {  // 날아가는 공격 (선택). 틀리면 공책에 적혀서 아래에서 거절돼요
        attack.projectile = read_projectile(reader, file, errors);
    }
    read_optional_path(reader, "fx", attack.fx);
    read_optional_path(reader, "sfx", attack.sfx);
    if (reader.has("next_combo")) {
        attack.next_combo = reader.string("next_combo", kMaxIdLength);
    }

    if (!id || !windup || !active || !recovery || !cancel || !hitbox || !power || !knockback ||
        !hitstop || !shake || !errors.empty()) {
        if (errors.empty()) {
            errors.add(file, "", "attack is incomplete");
        }
        return {std::nullopt, errors.take()};
    }
    attack.id = std::move(*id);
    attack.windup = static_cast<float>(*windup);
    attack.active = static_cast<float>(*active);
    attack.recovery = static_cast<float>(*recovery);
    attack.cancel_start = static_cast<float>(cancel->min);
    attack.cancel_end = static_cast<float>(cancel->max);
    attack.hitbox = *hitbox;
    attack.power = static_cast<float>(*power);
    attack.knockback = static_cast<float>(*knockback);
    attack.hitstop = static_cast<float>(*hitstop);
    attack.shake = static_cast<float>(*shake);

    // 취소 구간은 공격이 끝나기 전이어야 해요. 끝난 뒤에는 취소할 공격이 없어요.
    if (cancel->max > *windup + *active + *recovery + 1e-6) {
        reader.error("cancel_window", "must end before the attack ends (" +
                                          std::to_string(attack.total_seconds()) + " s)");
        return {std::nullopt, errors.take()};
    }
    return {std::move(attack), {}};
}

LoadResult<combat::AttackData> load_attack_file(const std::filesystem::path& content_root,
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
    return parse_attack(*document, name);
}

}  // namespace wind::content
