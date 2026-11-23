#pragma once

// 이 파일은 무엇을 하나요?
// 게임에 있는 모든 아이템 설계도를 모은 "도감"이에요. (명세 10.1, 14a)
// id로 설계도를 찾아요. 같은 id가 두 번 들어갈 수는 없어요.
// 파일에서 읽는 일은 item_catalog_loader가 하고, 이것은 찾기만 해요.

#include "wind/items/item_def.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace wind::items {

class ItemCatalog {
public:
    ItemCatalog() = default;

    // 설계도를 넣어요. 같은 id가 이미 있으면 넣지 않고 false예요.
    bool add(ItemDef def);
    // id로 찾아요. 없으면 nullptr (모르는 아이템)
    [[nodiscard]] const ItemDef* find(std::string_view id) const;
    [[nodiscard]] std::size_t size() const { return defs_.size(); }
    // 넣은 순서대로의 id 목록 (정렬 순서에 써요: 도감에 먼저 적힌 것이 앞)
    [[nodiscard]] const std::vector<std::string>& order() const { return order_; }
    // 도감에서 몇 번째로 적혔는지. 없으면 size() (맨 뒤로 보내요)
    [[nodiscard]] std::size_t rank(std::string_view id) const;

private:
    std::map<std::string, ItemDef, std::less<>> defs_;  // id → 설계도
    std::map<std::string, std::size_t, std::less<>> ranks_;  // id → 적힌 순서
    std::vector<std::string> order_;  // 적힌 순서대로의 id
};

}  // namespace wind::items
