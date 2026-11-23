// 이 파일은 무엇을 하나요?
// 아이템 도감 읽기, 빈 가방·바닥 보관함 만들기, 시험용 가방 채우기.

#include "app/item_setup.hpp"

#include "wind/content/item_catalog_loader.hpp"
#include "wind/log.hpp"

#include <string>
#include <utility>

namespace wind::client {

std::unique_ptr<ItemSetup> load_item_setup(const std::filesystem::path& content_root,
                                           std::string_view fill_bag_item) {
    auto catalog = content::load_item_catalog(content_root);
    if (!catalog.ok()) {
        content::log_content_errors(content::kItemCatalogPath, catalog.errors);  // 어디가 왜 틀렸는지
        return nullptr;
    }
    auto setup = std::make_unique<ItemSetup>();
    setup->catalog = std::move(*catalog.value);
    log::info("Item catalog: {} items", setup->catalog.size());
    if (fill_bag_item.empty()) return setup;
    // 시험용: 빈 칸이 없어질 때까지 한 묶음(max_stack개)씩 넣어요
    const auto* def = setup->catalog.find(fill_bag_item);
    if (def == nullptr) {
        log::warn("--fill-bag: unknown item {}; starting with an empty bag", fill_bag_item);
        return setup;
    }
    while (setup->bag.try_add(setup->catalog, def->id, def->max_stack)) {
    }
    log::info("--fill-bag: bag filled with {} ({} free slots)", def->id, setup->bag.free_slots());
    return setup;
}

}  // namespace wind::client
