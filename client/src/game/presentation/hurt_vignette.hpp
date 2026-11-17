#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 맞았을 때 화면 가장자리가 붉게 물들었다가 사라지는 연출이에요. (명세 6.7, 11c)
// - 맞으면 세기가 1이 되고, 0.6초 동안 줄어들어요 (처음엔 빨리, 끝은 천천히 사라지게).
// - 가운데는 투명하고 가장자리로 갈수록 진한 그림(비네트 그림)을 붉게 칠해서 화면 전체에 덮어요.
// - 조명 뒤(Foreground)에 그려서 밤에도 보여요. 머리 위 HP 바·느낌표보다는 아래예요.
// - 접근성: 번쩍임 세기 설정(--flash)을 곱해요. 0이면 안 보여요.
// 보여 주기만 하고 게임 결과는 바꾸지 않아요. 값의 이유는 docs/VISUALS.md "Hero getting hit" 절에 있어요.

#include "engine/core/math.hpp"
#include "engine/render/render_queue.hpp"

namespace wind::game {

class HurtVignette {
public:
    // 맞았어요: 세기를 1로
    void trigger() { level_ = 1.0f; }
    // 화면 한 장마다: 세기를 줄여요
    void update(float dt_seconds);
    // 접근성 세기 0~1 (--flash)
    void set_strength(float strength);
    // 지금 진하기 0~1 (세기 설정을 곱한 값)
    [[nodiscard]] float alpha() const;
    // texture: 비네트 그림, viewport: 화면 크기 (640x360). 진하기가 0이면 아무것도 안 넣어요.
    void queue(engine::TextureHandle texture, engine::Vec2 viewport, engine::RenderQueue& queue) const;

private:
    float level_ = 0.0f;     // 남은 세기 1 -> 0
    float strength_ = 1.0f;  // 접근성 세기
};

}  // namespace wind::game
