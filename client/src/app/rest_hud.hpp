#pragma once

// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 글자 두 가지를 보여 줘요. (명세 8.7, 13b)
// - "[E] 쉬기": 쿠션 가까이 서 있으면 화면 아래 가운데에 살짝 떠올라요 (나타나고 사라질 때 부드럽게).
// - "주변이 너무 소란스럽다": 몬스터가 노리거나 방금 맞아서 못 쉬면 1.8초 동안 보여요.
// 게임 규칙은 바꾸지 않고, 필드가 알려 준 값만 보여 줘요.

#include "engine/core/math.hpp"

#include <optional>

namespace wind::engine {
class UiPainter;
}

namespace wind::client {

class RestHud {
public:
    // 화면 한 장마다: prompt = 안내를 보여 줄 때인가, denied = 못 쉰 횟수 (늘면 알림)
    void update(float dt_seconds, bool prompt, unsigned denied);
    void draw(engine::UiPainter& painter, engine::Vec2 screen) const;

    // 안내 투명도 0~1 (테스트·그리기용)
    [[nodiscard]] float prompt_alpha() const { return prompt_alpha_; }
    // 알림이 보이는 중인가요?
    [[nodiscard]] bool notice_visible() const { return notice_seconds_ > 0.0f; }

private:
    float prompt_alpha_ = 0.0f;               // "[E] 쉬기" 투명도 (서서히 바뀌어요)
    float notice_seconds_ = 0.0f;             // 알림이 보일 남은 시간
    std::optional<unsigned> denied_seen_;     // 마지막으로 본 "못 쉼" 횟수 (처음엔 없음)
};

}  // namespace wind::client
