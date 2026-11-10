#pragma once

// 이 파일은 무엇을 하나요?
// 안개 층이에요. (명세 6.5)
// 크고 흐릿한 하얀 구름 덩어리 몇 개가 화면 위를 천천히 옆으로 흘러가요.
// 짙기(fog, 0~1)가 0이면 안 보이고, 1에 가까울수록 진해져요.

#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"

namespace wind::game {

class FogLayer {
public:
    void update(float dt_seconds) { time_ += dt_seconds; }
    // fog: 짙기 0~1, texture: 부드러운 둥근 그림 (빛 그림을 같이 써요)
    void queue(const engine::Camera2D& camera, float fog, engine::TextureHandle texture,
               engine::RenderQueue& queue) const;

private:
    float time_ = 0.0f;
};

}  // namespace wind::game
