// 이 파일은 무엇을 하나요?
// 아이템 도감 읽기, 빈 가방·바닥 보관함 만들기, 무기 선반과 시작 무기, 시험용 가방 채우기.

#include "app/item_setup.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/log.hpp"

#include <string>
#include <utility>

namespace wind::client {
namespace {

// 시작 무기 (명세 8.1). 저장 파일이 생기면(16단계) 저장된 장비를 껴요.
constexpr std::string_view kStartWeapon = "longsword";

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

}  // namespace

std::unique_ptr<ItemSetup> load_item_setup(const std::filesystem::path& content_root,
                                           std::string_view fill_bag_item,
                                           const std::vector<std::string>& give_items) {
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
    give(*setup, give_items);  // --give를 먼저 넣어야 --fill-bag이 남은 칸만 채워요
    if (!fill_bag_item.empty()) fill_bag(*setup, fill_bag_item);
    return setup;
}

}  // namespace wind::client
