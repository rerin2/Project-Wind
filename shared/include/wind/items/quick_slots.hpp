#pragma once

// 이 파일은 무엇을 하나요?
// 단축 아이템 칸 1~4예요. (명세 13.2, 13.4, 15f)
// - 칸마다 아이템 id 하나만 기억해요 (아이템 자체는 가방에 그대로 있어요).
// - 등록(assign): 쓸 수 있는 아이템(소비, use가 있음)만 돼요 (개발자와 정함).
//   다른 칸에 이미 있으면 그 칸에서 빼고 이 칸으로 옮겨요. 이미 이 칸에 있으면 등록을 풀어요(같은 키를 한 번 더).
// - 다 써서 가방에 0개가 돼도 등록은 남아요 (개발자와 정함). 다시 주우면 바로 쓸 수 있어요.
// - 쓰기: stack_for가 가방에서 그 아이템이 든 첫 칸을 찾아 주고, 실제로 마시는 일은 필드 규칙(14d)이 해요.
// 저장 파일에는 칸마다 id를 적어요 (16단계). 게임 규칙이라 SDL을 몰라요.

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace wind::items {

class Inventory;
class ItemCatalog;

class QuickSlots {
public:
    static constexpr std::size_t kCount = 4;  // 1~4 키

    enum class Result : std::uint8_t {
        Assigned,   // 이 칸에 등록했어요 (다른 칸에 있었으면 옮겼어요)
        Cleared,    // 이미 이 칸에 있어서 등록을 풀었어요
        NotUsable,  // 쓸 수 없는 아이템이거나 도감에 없어요: 아무것도 안 바뀌어요
        BadSlot,    // 칸 번호가 4 이상이에요: 아무것도 안 바뀌어요
    };

    Result assign(std::size_t slot, std::string_view item_id, const ItemCatalog& catalog);
    void clear(std::size_t slot);
    // 이 칸의 아이템 id (비었으면 빈 글자)
    [[nodiscard]] const std::string& at(std::size_t slot) const;
    // 이 아이템이 든 칸 번호 (없으면 nullopt). 가방 화면이 칸에 작은 번호를 붙여요.
    [[nodiscard]] std::optional<std::size_t> slot_of(std::string_view item_id) const;
    // 이 칸의 아이템이 든 가방의 첫 칸 (비었거나 가방에 없으면 nullopt)
    [[nodiscard]] std::optional<std::size_t> stack_for(std::size_t slot, const Inventory& bag) const;

private:
    std::array<std::string, kCount> items_;
};

}  // namespace wind::items
