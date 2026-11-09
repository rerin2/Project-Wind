#pragma once

// 이 파일은 무엇을 하나요?
// 트윈(tween) = 숫자 하나를 "시작 값 → 끝 값"으로 정해진 시간 동안 부드럽게 바꿔 주는 도구예요.
// 예: 투명도를 0.3초 동안 255 → 0으로 (화면 번쩍임이 사라지기),
//     창 위치를 0.2초 동안 -40 → 0으로 (UI 창이 위에서 쓱 내려오기)
// 어떤 느낌으로 바뀔지는 이징(Ease)이 정해요.

#include "engine/tween/easing.hpp"

namespace wind::engine {

class Tween {
public:
    Tween() = default;
    Tween(float from, float to, float duration_seconds, Ease ease);

    // 처음부터 다시 시작해요.
    void restart();
    // 시간이 dt초 흘렀어요.
    void update(float dt_seconds);

    // 지금 값
    [[nodiscard]] float value() const;
    // 끝까지 갔나요? (길이가 0인 트윈은 처음부터 끝난 상태예요)
    [[nodiscard]] bool finished() const { return elapsed_ >= duration_; }
    // 진행률 0~1
    [[nodiscard]] float progress() const;

private:
    float from_ = 0.0f;
    float to_ = 0.0f;
    float duration_ = 0.0f;
    float elapsed_ = 0.0f;
    Ease ease_ = Ease::Linear;
};

}  // namespace wind::engine
