#pragma once

// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 "마술 칸" 화면의 생각하는 부분이에요. (명세 8.6, 8.7, 15f) 그리기는 app 층(spell_slot_view)이 해요.
// - 1단계 칸 고르기: 위·아래로 열린 칸 중 하나 (잠긴 칸은 못 골라요). 확인 → 2단계, 뒤로 → 쿠션 메뉴로
// - 2단계 마술 고르기: 배운 마술 목록 + 맨 아래 "비우기". 확인하면
//     마술: SpellBook::place (요구 지능 확인, 다른 칸에 있으면 두 칸을 맞바꿈, 개발자와 정함)
//     비우기: 그 칸을 비워요
//   되면 1단계로 돌아가고 Changed, 지능이 모자라면 아무것도 안 바뀌고 Denied (화면이 흔들려요).
//   뒤로 → 1단계로.
// 키 대신 MenuInput을 받아서 창 없이 테스트할 수 있어요.

#include "game/ui/menu_input.hpp"

#include <cstddef>
#include <cstdint>

namespace wind::game {

class HeroProgress;
class SpellShelf;

class SpellSlotSheet {
public:
    enum class Phase : std::uint8_t { Slots, Picking };
    enum class Result : std::uint8_t {
        None,     // 화면 안에서만 바뀌었어요
        Changed,  // 칸이 바뀌었어요: 필드가 주인공 손의 마술을 다시 쥐여 줘요
        Denied,   // 요구 지능이 모자라 넣지 못했어요 (아무것도 안 바뀜)
        Back,     // 쿠션 메뉴로 돌아가요
    };

    // 처음 상태로 (1단계, 첫 칸)
    void reset();
    Result handle(MenuInput input, HeroProgress& progress, const SpellShelf& shelf);

    [[nodiscard]] Phase phase() const { return phase_; }
    [[nodiscard]] std::size_t slot() const { return slot_; }
    // 2단계에서 고른 줄 (배운 마술 순서, 마지막 줄은 "비우기")
    [[nodiscard]] std::size_t pick() const { return pick_; }
    // 지금 열린 칸 수 (지능 곡선, 최대 4)
    [[nodiscard]] static std::size_t open_slots(const HeroProgress& progress);

private:
    Result handle_slots(MenuInput input, const HeroProgress& progress);
    Result handle_picking(MenuInput input, HeroProgress& progress, const SpellShelf& shelf);

    Phase phase_ = Phase::Slots;
    std::size_t slot_ = 0;
    std::size_t pick_ = 0;
};

}  // namespace wind::game
