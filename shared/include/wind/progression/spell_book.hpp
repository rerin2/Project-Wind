#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 마술 공책이에요. (명세 8.6, 15b)
// - 배운 마술 목록 (마술 id, 배운 순서)
// - 마술 칸: 최대 4칸. 실제로 쓸 수 있는 칸 수는 지능 곡선(derive_stats의 spell_slots, 2~4칸)이 정해요.
//   칸 수보다 뒤에 있는 칸은 "잠긴 칸"이라 비어 있는 것처럼 다뤄요(지능이 오르면 열려요).
// - 고른 칸: 필드에서 Q로 다음 칸(빈 칸은 건너뛰어요)으로 넘겨요. L/C를 누르면 고른 칸의 마술이 나가요.
// 규칙: 배운 마술만, 요구 지능 이상일 때만 칸에 넣을 수 있어요(테스트). 같은 마술을 두 칸에 넣을 수 없어요.
// 실패하면 아무것도 안 바뀌어요. 마술 데이터(요구 지능)는 부르는 쪽이 넘겨줘요(파일을 읽지 않는 순수 규칙).
// 저장 파일에는 배운 목록·칸·고른 칸을 적어요(16단계).

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace wind::progression {

class SpellBook {
public:
    static constexpr std::size_t kMaxSlots = 4;  // 마술 칸 최대 (명세 8.2 곡선의 끝값)

    // 칸에 넣기 결과 (Done이 아니면 아무것도 안 바뀌었어요)
    enum class SlotResult : std::uint8_t {
        Done,
        NotLearned,         // 배우지 않은 마술
        NeedsIntelligence,  // 요구 지능보다 지능이 낮아요
        LockedSlot,         // 지금 칸 수 밖의 칸이에요
        AlreadySlotted,     // 다른 칸에 이미 있어요
    };

    // 마술을 배워요. 이미 배웠거나 id가 비었으면 false
    bool learn(std::string id);
    [[nodiscard]] bool knows(std::string_view id) const;
    [[nodiscard]] const std::vector<std::string>& learned() const { return learned_; }

    // slot 칸에 마술을 넣어요. intelligence: 지금 지능, required: 그 마술의 요구 지능, open_slots: 지금 칸 수
    SlotResult put(std::size_t slot, std::string_view id, int required, int intelligence, std::size_t open_slots);
    // 열린 칸 중 맨 앞 빈 칸에 넣어요 (배울 때 저절로, 개발자와 정함). 빈 칸이 없으면 LockedSlot
    SlotResult put_in_free_slot(std::string_view id, int required, int intelligence, std::size_t open_slots);
    // 칸을 비워요 (15f 쿠션에서 씀)
    void clear(std::size_t slot);
    // 이 칸의 마술 id (비었거나 잠긴 칸이면 빈 글자)
    [[nodiscard]] std::string_view in_slot(std::size_t slot, std::size_t open_slots) const;

    // 고른 칸에 든 마술 (고른 칸이 비었거나 잠겼으면 열린 칸 중 맨 앞 마술, 하나도 없으면 빈 글자)
    [[nodiscard]] std::string_view selected(std::size_t open_slots) const;
    // 고른 칸 번호 (selected와 같은 규칙. 마술이 하나도 없으면 0)
    [[nodiscard]] std::size_t selected_slot(std::size_t open_slots) const;
    // Q: 다음 마술이 든 칸으로 (끝이면 처음으로). 다른 칸으로 바뀌었으면 true (마술이 하나뿐이면 false)
    bool select_next(std::size_t open_slots);

private:
    [[nodiscard]] static std::size_t open_count(std::size_t open_slots);

    std::vector<std::string> learned_;                 // 배운 마술 id (배운 순서)
    std::array<std::string, kMaxSlots> slots_;         // 칸마다 마술 id (비었으면 빈 글자)
    std::size_t selected_ = 0;                         // 고른 칸
};

}  // namespace wind::progression
