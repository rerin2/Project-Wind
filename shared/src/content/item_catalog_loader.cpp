// 이 파일은 무엇을 하나요?
// 아이템 도감 파일을 아이템마다 한 칸씩 확인하며 읽어요.

#include "wind/content/item_catalog_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"
#include "wind/content/weapon_loader.hpp"

#include <optional>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxItems = 256;               // 도감 하나의 아이템 상한
constexpr std::size_t kMaxIdLength = 64;
constexpr std::size_t kMaxNameLength = 64;           // UTF-8 바이트 (한글 한 글자 = 3바이트)
constexpr std::size_t kMaxDescriptionLength = 256;   // 한글 약 80자
constexpr std::int64_t kMaxStack = 999;              // 한 칸에 쌓이는 개수 상한
constexpr std::int64_t kMaxIcon = 255;               // 아이콘 칸 번호 상한
constexpr std::int64_t kMaxHeal = 99999;             // 회복량 상한 (HP 상한과 같아요)

// "consumable" 같은 글자를 종류로 바꿔요. 모르는 글자면 nullopt
std::optional<items::ItemKind> kind_from(std::string_view text) {
    if (text == "consumable") return items::ItemKind::Consumable;
    if (text == "material") return items::ItemKind::Material;
    if (text == "weapon") return items::ItemKind::Weapon;
    return std::nullopt;
}

// "common" 같은 글자를 희귀도로 바꿔요. 모르는 글자면 nullopt
std::optional<items::Rarity> rarity_from(std::string_view text) {
    if (text == "common") return items::Rarity::Common;
    if (text == "uncommon") return items::Rarity::Uncommon;
    if (text == "rare") return items::Rarity::Rare;
    return std::nullopt;
}

// "use": { "heal_hp": 50, "heal_mp": 0 } 읽기. 둘 다 0이면 아무 일도 안 하는 물약이라 거절해요.
std::optional<items::UseEffect> read_use(JsonReader& item) {
    const auto* object = item.object("use");
    if (object == nullptr) return std::nullopt;
    JsonReader use(*object, item.file(), item.field_path("use"), item.errors());
    if (!use.valid()) return std::nullopt;
    items::UseEffect effect;
    // 두 칸 모두 선택이에요 (없으면 0)
    if (use.has("heal_hp")) {
        const auto hp = use.integer("heal_hp", 0, kMaxHeal);
        if (!hp) return std::nullopt;
        effect.heal_hp = static_cast<std::int32_t>(*hp);
    }
    if (use.has("heal_mp")) {
        const auto mp = use.integer("heal_mp", 0, kMaxHeal);
        if (!mp) return std::nullopt;
        effect.heal_mp = static_cast<std::int32_t>(*mp);
    }
    if (effect.heal_hp == 0 && effect.heal_mp == 0) {
        use.error(item.field_path("use"), "needs heal_hp or heal_mp above 0");
        return std::nullopt;
    }
    return effect;
}

// 종류마다 다른 칸(use, weapon)을 읽고 검사해요. 틀리면 공책에 적고 false.
bool read_kind_fields(JsonReader& item, items::ItemDef& def, const WeaponExists& weapon_exists) {
    const bool consumable = def.kind == items::ItemKind::Consumable;
    const bool weapon = def.kind == items::ItemKind::Weapon;
    // use는 소비 아이템에만 있어야 해요
    if (consumable) {
        def.use = read_use(item);
        if (!def.use) return false;
    } else if (item.has("use")) {
        item.error(item.field_path("use"), "only consumable items can have use");
        return false;
    }
    // weapon은 무기에만 있어야 해요
    if (!weapon) {
        if (item.has("weapon")) {
            item.error(item.field_path("weapon"), "only weapon items can have weapon");
            return false;
        }
        return true;
    }
    auto path = item.content_path("weapon");
    if (!path) return false;
    if (!weapon_exists(*path)) {  // 참조 검사: 가리키는 무기 파일이 진짜로 읽혀야 해요
        item.error(item.field_path("weapon"), "weapon file failed to load: " + *path);
        return false;
    }
    if (def.max_stack != 1) {  // 무기는 한 칸에 하나씩만
        item.error(item.field_path("max_stack"), "weapon items must have max_stack 1");
        return false;
    }
    def.weapon = std::move(*path);
    def.equip = items::EquipSlot::Weapon;
    return true;
}

// 아이템 하나 읽기. 틀리면 공책에 적고 nullopt.
std::optional<items::ItemDef> read_item(JsonReader& item, const WeaponExists& weapon_exists) {
    if (!item.valid()) return std::nullopt;  // { ... }가 아니면 읽을 수 없어요
    auto id = item.string("id", kMaxIdLength);
    auto name = item.string("name", kMaxNameLength);
    auto description = item.string("description", kMaxDescriptionLength);
    const auto kind_text = item.string("kind", kMaxIdLength);
    const auto max_stack = item.integer("max_stack", 1, kMaxStack);
    const auto icon = item.integer("icon", 0, kMaxIcon);
    // 희귀도는 선택 칸이에요 (없으면 common)
    std::optional<items::Rarity> rarity = items::Rarity::Common;
    if (item.has("rarity")) {
        const auto text = item.string("rarity", kMaxIdLength);
        rarity = text ? rarity_from(*text) : std::nullopt;
        if (text && !rarity) item.error(item.field_path("rarity"), "unknown rarity: " + *text);
    }
    const auto kind = kind_text ? kind_from(*kind_text) : std::nullopt;
    if (kind_text && !kind) item.error(item.field_path("kind"), "unknown item kind: " + *kind_text);
    if (!id || !name || !description || !kind || !max_stack || !icon || !rarity) return std::nullopt;

    items::ItemDef def;
    def.id = std::move(*id);
    def.name = std::move(*name);
    def.description = std::move(*description);
    def.kind = *kind;
    def.max_stack = static_cast<std::int32_t>(*max_stack);
    def.icon = static_cast<std::int32_t>(*icon);
    def.rarity = *rarity;
    if (!read_kind_fields(item, def, weapon_exists)) return std::nullopt;
    return def;
}

}  // namespace

LoadResult<items::ItemCatalog> parse_item_catalog(const nlohmann::json& document,
                                                  const std::string& file,
                                                  const WeaponExists& weapon_exists) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    if (!reader.valid() || !reader.schema_version(kItemCatalogSchemaVersion)) {
        if (errors.empty()) errors.add(file, "", "expected a JSON object");
        return {std::nullopt, errors.take()};
    }
    const auto* list = reader.array("items", kMaxItems);  // 없거나 너무 많으면 공책에 적혀요
    if (list == nullptr) return {std::nullopt, errors.take()};
    if (list->empty()) {
        reader.error("items", "needs at least one item");
        return {std::nullopt, errors.take()};
    }

    items::ItemCatalog catalog;
    for (std::size_t i = 0; i < list->size() && !errors.full(); ++i) {
        const auto field = reader.index_path("items", i);
        // 아이템 하나를 읽는 도우미 ({ ... }가 아니면 만들 때 공책에 적어요)
        JsonReader item(list->at(i), file, field, errors);
        auto def = read_item(item, weapon_exists);
        if (!def) continue;  // 이유는 이미 공책에 적혔어요
        const std::string id = def->id;
        if (!catalog.add(std::move(*def))) errors.add(file, field + ".id", "duplicate item id: " + id);
    }
    if (!errors.empty()) return {std::nullopt, errors.take()};
    return {std::move(catalog), {}};
}

LoadResult<items::ItemCatalog> load_item_catalog(const std::filesystem::path& content_root) {
    ErrorList errors;
    const std::string name(kItemCatalogPath);
    const auto file = resolve_content_path(content_root, kItemCatalogPath);
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = read_json_file(*file, name, errors);
    if (!document) return {std::nullopt, errors.take()};
    // 무기 참조는 진짜 무기 파일(공격 파일까지)을 읽어 봐요
    const WeaponExists from_disk = [&content_root](std::string_view path) {
        return load_weapon_file(content_root, path).ok();
    };
    return parse_item_catalog(*document, name, from_disk);
}

}  // namespace wind::content
