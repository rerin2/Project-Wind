#pragma once

// 이 파일은 무엇을 하나요?
// 화면 전환 페이드예요. (명세 6.8)
// 맵을 옮길 때: 화면이 서서히 까매지고(어두워지기) -> 까만 동안 새 맵을 불러오고
//              -> 다시 서서히 밝아져요(밝아지기).
// 그리는 일은 하지 않고 "지금 얼마나 까만지"(0~1)만 계산해요. 그래서 창 없이 테스트할 수 있어요.

#include "engine/tween/tween.hpp"

namespace wind::engine {

class ScreenFade {
public:
    enum class Phase { Clear, FadingOut, Covered, FadingIn };

    // seconds: 어두워지기(또는 밝아지기)에 걸리는 시간
    explicit ScreenFade(float seconds);

    // 어두워지기 시작. 다 까매지면 Covered가 돼요.
    void fade_out();
    // 밝아지기 시작. 다 밝아지면 Clear가 돼요.
    void fade_in();
    void update(float dt_seconds);

    [[nodiscard]] Phase phase() const { return phase_; }
    // 화면을 덮는 검은색 진하기 0(안 보임) ~ 1(완전히 까맘)
    [[nodiscard]] float darkness() const;
    // 페이드 중이거나 덮여 있나요? (이때는 게임 입력을 막아요)
    [[nodiscard]] bool busy() const { return phase_ != Phase::Clear; }

private:
    float seconds_;
    Phase phase_ = Phase::Clear;
    Tween tween_;
};

}  // namespace wind::engine
