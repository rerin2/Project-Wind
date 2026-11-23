#pragma once

// 이 파일은 무엇을 하나요?
// 필드의 마술 담당이에요. (15b) 주인공 손에 "지금 고른 마술 칸의 마술"을 쥐여 줘요.
// - arm: 마술 공책의 고른 마술 id → 마술 선반에서 마술 → 주인공 시전 담당에 넣어요 (없으면 마술 없음).
//   필드를 만들 때, 레벨업 뒤(지능이 오르면 칸 수가 늘어요)에 불러요.
// - Q: 다음 마술이 든 칸으로 바꿔요. 시전 중에 누른 Q는 무시해요(개발자와 정함). 다음 박자에 처리해요.
// 마술 공책은 진행 상태(HeroProgress)에, 마술 선반은 필드 밖(ClientApp)에 있고 여기서는 빌려 써요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "wind/combat/spell.hpp"

#include <cstddef>

namespace wind::game {

class HeroProgress;
class PlayerMotion;
class SpellShelf;

class FieldSpells {
public:
    FieldSpells(HeroProgress& progress, const SpellShelf& shelf) : progress_(progress), shelf_(shelf) {}

    // 지금 고른 마술 (마술 칸이 비었거나 선반에 없으면 nullptr)
    [[nodiscard]] const combat::Spell* selected() const;
    // 주인공 시전 담당에 고른 마술을 넣어요
    void arm(PlayerMotion& hero) const;
    // Q를 눌렀어요 (화면 한 장마다, 박자 계산 전에)
    void press_next() { next_pressed_ = true; }
    // 정해진 박자마다: Q를 처리해요. 시전 중이면 버려요. 바뀌었으면 true
    bool step(PlayerMotion& hero);
    // Q로 마술이 바뀐 횟수 (늘기만 해요. HUD가 바뀐 걸 보고 아이콘을 톡 튀게 해요)
    [[nodiscard]] unsigned switch_count() const { return switch_count_; }

private:
    [[nodiscard]] std::size_t open_slots() const;

    HeroProgress& progress_;   // 마술 공책과 지능 (빌려 씀)
    const SpellShelf& shelf_;  // 마술 데이터 (빌려 씀)
    bool next_pressed_ = false;
    unsigned switch_count_ = 0;
};

}  // namespace wind::game
