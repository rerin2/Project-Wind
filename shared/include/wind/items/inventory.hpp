#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 가방(인벤토리) 규칙이에요. (명세 10.2, 14a)
// - 칸으로 되어 있어요 (기본 24칸). 칸 하나에는 같은 아이템이 max_stack개까지 쌓여요.
// - 넣기·빼기는 "전부 성공 또는 전부 실패": 다 못 넣거나 다 못 빼면 한 칸도 안 바뀌어요.
//   그래서 가득 찬 가방에 넣으려던 아이템은 바닥에 그대로 남아요 (14b).
// - 정렬: 같은 아이템을 한 칸으로 모으고 도감 순서대로 앞에서부터 놓아요.
// 장착·해제는 15단계에서 더해요. 게임 규칙이라 SDL을 몰라요.

#include "wind/items/item_stack.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace wind::items {

class ItemCatalog;

class Inventory {
public:
    static constexpr std::size_t kDefaultSlots = 24;  // 명세 10.2 기본 칸 수

    // slots: 칸 수 (0이면 아무것도 못 넣는 가방)
    explicit Inventory(std::size_t slots = kDefaultSlots);

    [[nodiscard]] std::size_t slot_count() const { return slots_.size(); }
    // 칸 하나 보기. 비었거나 칸 번호가 밖이면 nullopt
    [[nodiscard]] const std::optional<ItemStack>& at(std::size_t slot) const;
    // 이 아이템을 모두 몇 개 가졌나요?
    [[nodiscard]] std::int64_t count_of(std::string_view item_id) const;
    // 빈 칸 수
    [[nodiscard]] std::size_t free_slots() const;

    // count개를 모두 넣을 수 있나요? (도감에 없는 아이템이거나 count가 1보다 작으면 false)
    [[nodiscard]] bool can_add(const ItemCatalog& catalog, std::string_view item_id,
                               std::int32_t count) const;
    // count개를 넣어요: 같은 아이템 칸을 먼저 채우고, 남으면 앞쪽 빈 칸부터 써요.
    // 다 넣을 수 없으면 아무것도 바꾸지 않고 false예요.
    bool try_add(const ItemCatalog& catalog, std::string_view item_id, std::int32_t count);
    // count개를 빼요: 뒤쪽 칸부터 줄여요. 모자라면 아무것도 바꾸지 않고 false예요.
    bool try_remove(std::string_view item_id, std::int32_t count);
    // 칸 하나에서 count개를 꺼내 돌려줘요 (버리기·쓰기에 써요).
    // 칸이 비었거나, 번호가 밖이거나, count가 1보다 작거나 가진 것보다 많으면 아무것도 안 바뀌고 nullopt예요.
    std::optional<ItemStack> take_at(std::size_t slot, std::int32_t count);
    // 빈 칸 하나에 묶음을 그대로 놓아요 (장비 바꿔 끼우기: 벗은 무기를 꺼낸 자리에, 15a).
    // 칸이 차 있거나, 번호가 밖이거나, 개수가 1보다 작으면 아무것도 안 바뀌고 false예요.
    bool place_at(std::size_t slot, ItemStack stack);
    // 같은 아이템을 모으고 도감 순서대로 앞에서부터 놓아요. 도감에 없는 아이템은 맨 뒤에 그대로 둬요.
    // (칸이 모자랄 일은 없지만, 혹시 모자라면 아무것도 바꾸지 않고 false예요)
    bool sort(const ItemCatalog& catalog);

private:
    // 이 아이템을 몇 개 더 넣을 수 있는지 (같은 아이템 칸의 남은 자리 + 빈 칸 × max_stack)
    [[nodiscard]] std::int64_t room_for(std::string_view item_id, std::int32_t max_stack) const;

    std::vector<std::optional<ItemStack>> slots_;  // 칸들 (비었으면 nullopt)
};

}  // namespace wind::items
