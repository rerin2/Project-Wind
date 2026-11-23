// 이 파일은 무엇을 하나요?
// 아이템 도감: 설계도 넣기와 id로 찾기.

#include "wind/items/item_catalog.hpp"

#include <utility>

namespace wind::items {

bool ItemCatalog::add(ItemDef def) {
    if (defs_.contains(def.id)) return false;  // 같은 id는 두 번 넣지 않아요
    const std::string id = def.id;
    ranks_.emplace(id, order_.size());  // 지금까지 넣은 개수가 곧 순서예요
    order_.push_back(id);
    defs_.emplace(id, std::move(def));
    return true;
}

const ItemDef* ItemCatalog::find(std::string_view id) const {
    const auto it = defs_.find(id);
    return it == defs_.end() ? nullptr : &it->second;
}

std::size_t ItemCatalog::rank(std::string_view id) const {
    const auto it = ranks_.find(id);
    return it == ranks_.end() ? order_.size() : it->second;
}

}  // namespace wind::items
