// 이 파일은 무엇을 하나요?
// 아이템 도감 읽기, 빈 가방·바닥 보관함 만들기, 무기 선반과 시작 무기, 시작 마술, 시험용 가방 채우기·마술 배우기.

#include "app/item_setup.hpp"

#include "app/client_options.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/progression/stats.hpp"
#include "wind/log.hpp"

#include <string>
#include <utility>

namespace wind::client {
namespace {

// 시작 무기 (명세 8.1). 저장 파일이 생기면(16단계) 저장된 장비를 껴요.
constexpr std::string_view kStartWeapon = "longsword";
// 시작 마술 (명세 8.1). 첫 마술 칸에 넣어요.
constexpr std::string_view kStartSpell = "star_shard";

// 시험용 --fill-bag: 빈 칸이 없어질 때까지 한 묶음(max_stack개)씩 넣어요
void fill_bag(ItemSetup& setup, std::string_view item_id) {
    const auto* def = setup.catalog.find(item_id);
    if (def == nullptr) {
        log::warn("--fill-bag: unknown item {}; starting with an empty bag", item_id);
        return;
    }
    while (setup.bag.try_add(setup.catalog, def->id, def->max_stack)) {
    }
    log::info("--fill-bag: bag filled with {} ({} free slots)", def->id, setup.bag.free_slots());
}

// 시험용 --give (15a): 아이템을 하나씩 넣어요. 모르는 아이템이거나 가방이 가득이면 경고만 해요
void give(ItemSetup& setup, const std::vector<std::string>& item_ids) {
    for (const auto& id : item_ids) {
        if (setup.bag.try_add(setup.catalog, id, 1)) {
            log::info("--give: added {}", id);
        } else {
            log::warn("--give: could not add {} (unknown item or full bag)", id);
        }
    }
}

// 마술 하나를 배우고 빈 마술 칸에 넣어요 (시작 마술, 시험용 --learn, 15b).
// 칸에 못 넣으면(지능 모자람, 빈 칸 없음) 배우기만 하고 이유를 기록해요.
void learn_spell(const std::filesystem::path& content_root, std::string_view id, game::SpellShelf& shelf,
                 game::HeroProgress& progress) {
    if (!shelf.load(content_root, id)) return;  // 이유는 이미 기록했어요
    const combat::Spell& spell = *shelf.find(id);
    auto& book = progress.spells();
    (void)book.learn(spell.id);
    const int intelligence = progress.stats()[progression::StatId::Intelligence];
    const auto slots = static_cast<std::size_t>(progress.derived().spell_slots);
    const auto result = book.put_in_free_slot(spell.id, spell.intelligence_required, intelligence, slots);
    if (result == progression::SpellBook::SlotResult::Done) {
        log::info("Learned {} and put it in a spell slot", spell.id);
    } else if (result == progression::SpellBook::SlotResult::NeedsIntelligence) {
        log::warn("Learned {} but intelligence {} < {}; not slotted", spell.id, intelligence, spell.intelligence_required);
    } else {
        log::warn("Learned {} but no free spell slot ({} open)", spell.id, slots);
    }
}

}  // namespace

std::unique_ptr<ItemSetup> load_item_setup(const ClientOptions& options, game::HeroProgress& progress) {
    const std::filesystem::path& content_root = options.content_dir;
    auto catalog = content::load_item_catalog(content_root);
    if (!catalog.ok()) {
        content::log_content_errors(content::kItemCatalogPath, catalog.errors);  // 어디가 왜 틀렸는지
        return nullptr;
    }
    auto setup = std::make_unique<ItemSetup>();
    setup->catalog = std::move(*catalog.value);
    setup->weapons = game::WeaponRack::load(content_root, setup->catalog);  // 무기 파일들 (15a)
    log::info("Item catalog: {} items, {} weapons", setup->catalog.size(), setup->weapons.size());
    if (setup->weapons.find(kStartWeapon) != nullptr) {
        setup->worn.wear(items::EquipSlot::Weapon, std::string(kStartWeapon));
    } else {
        log::warn("Start weapon {} is missing; starting with bare hands", kStartWeapon);
    }
    give(*setup, options.give_items);  // --give를 먼저 넣어야 --fill-bag이 남은 칸만 채워요
    if (!options.fill_bag_item.empty()) fill_bag(*setup, options.fill_bag_item);
    // 마술 (15b): 시작 마술을 첫 칸에, 그다음 시험용 --learn 마술들을 빈 칸에
    learn_spell(content_root, kStartSpell, setup->spells, progress);
    for (const auto& id : options.learn_spells) learn_spell(content_root, id, setup->spells, progress);
    return setup;
}

}  // namespace wind::client
