// 이 파일은 무엇을 하나요?
// 무기 파일을 한 칸씩 확인하며 읽고, 콤보 공격 파일들을 불러와 순서를 검사해요.

#include "wind/content/weapon_loader.hpp"

#include "wind/content/attack_loader.hpp"
#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <set>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxIdLength = 64;
constexpr std::size_t kMaxNameLength = 64;  // UTF-8 바이트 (한글 한 글자 = 3바이트)
constexpr std::size_t kMaxComboLength = 8;
constexpr double kMaxWeight = 100.0;
constexpr double kMaxBasePower = 999.0;
constexpr double kMaxStrengthScaling = 5.0;
constexpr double kMaxStaminaPerAttack = 200.0;

// 콤보 목록의 공격 파일들을 읽어요. 실패한 파일의 오류도 그대로 공책에 옮겨 적어요.
void read_combo(JsonReader& reader, const AttackSource& load_attack, combat::Weapon& weapon) {
    const auto* list = reader.array("attacks", kMaxComboLength);
    if (list == nullptr) {
        return;
    }
    if (list->empty()) {
        reader.error("attacks", "needs at least one attack");
        return;
    }
    std::set<std::string> ids;
    for (std::size_t i = 0; i < list->size(); ++i) {
        const auto field = reader.index_path("attacks", i);
        const auto& item = (*list)[i];
        const auto path = item.is_string() ? normalize_content_path(item.get<std::string>())
                                           : std::nullopt;
        if (!path) {
            reader.error(field, "expected a path inside the content folder");
            continue;
        }
        auto loaded = load_attack(*path);
        if (!loaded.ok()) {
            reader.error(field, "attack file failed to load: " + *path);
            for (auto& e : loaded.errors) {
                reader.errors().add(std::move(e.file), std::move(e.field), std::move(e.message));
            }
            continue;
        }
        if (!ids.insert(loaded.value->id).second) {
            reader.error(field, "duplicate attack id in combo: " + loaded.value->id);
            continue;
        }
        weapon.combo.push_back(std::move(*loaded.value));
    }
}

// 각 공격의 next_combo가 목록 순서와 맞는지 봐요. 데이터끼리 말이 다르면 거절해요.
void check_combo_links(JsonReader& reader, const combat::Weapon& weapon) {
    for (std::size_t i = 0; i < weapon.combo.size(); ++i) {
        const auto& next = weapon.combo[i].next_combo;
        if (!next) {
            continue;
        }
        const bool last = i + 1 == weapon.combo.size();
        if (last) {
            reader.error(reader.index_path("attacks", i),
                         "last attack in the combo must not have next_combo (" + *next + ")");
        } else if (*next != weapon.combo[i + 1].id) {
            reader.error(reader.index_path("attacks", i),
                         "next_combo " + *next + " does not match the next attack " +
                             weapon.combo[i + 1].id);
        }
    }
}

}  // namespace

LoadResult<combat::Weapon> parse_weapon(const nlohmann::json& document, const std::string& file,
                                        const AttackSource& load_attack) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    if (!reader.valid() || !reader.schema_version(kWeaponSchemaVersion)) {
        if (errors.empty()) {
            errors.add(file, "", "expected a JSON object");
        }
        return {std::nullopt, errors.take()};
    }

    combat::Weapon weapon;
    auto id = reader.string("id", kMaxIdLength);
    auto name = reader.string("name", kMaxNameLength);
    const auto weight = reader.number("weight", 0.0, kMaxWeight);
    const auto base_power = reader.number("base_power", 0.0, kMaxBasePower);
    const auto scaling = reader.number("strength_scaling", 0.0, kMaxStrengthScaling);
    const auto stamina = reader.number("stamina_per_attack", 0.0, kMaxStaminaPerAttack);
    if (reader.has("crit_bonus")) {
        if (const auto crit = reader.number("crit_bonus", 0.0, 1.0)) {
            weapon.crit_bonus = static_cast<float>(*crit);
        }
    }
    read_combo(reader, load_attack, weapon);
    check_combo_links(reader, weapon);

    if (!id || !name || !weight || !base_power || !scaling || !stamina || !errors.empty()) {
        if (errors.empty()) {
            errors.add(file, "", "weapon is incomplete");
        }
        return {std::nullopt, errors.take()};
    }
    weapon.id = std::move(*id);
    weapon.name = std::move(*name);
    weapon.weight = static_cast<float>(*weight);
    weapon.base_power = static_cast<float>(*base_power);
    weapon.strength_scaling = static_cast<float>(*scaling);
    weapon.stamina_per_attack = static_cast<float>(*stamina);
    return {std::move(weapon), {}};
}

LoadResult<combat::Weapon> load_weapon_file(const std::filesystem::path& content_root,
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
    const AttackSource from_disk = [&content_root](std::string_view attack_path) {
        return load_attack_file(content_root, attack_path);
    };
    return parse_weapon(*document, name, from_disk);
}

}  // namespace wind::content
