// 이 파일은 무엇을 하나요?
// 빛을 그리기 명령으로 바꾸는 도우미를 숙제 검사(테스트)해요.
// - 화면 밖 빛은 건너뛰나, 한 화면 최대 개수를 지키나
// - 빛은 "더하기"로, 밝기는 진하기(a)로, 크기는 반지름 × 2로 그리나
#include "engine/light/light_drawing.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Camera2D;
using wind::engine::Light;
using wind::engine::LightingFrame;

Camera2D make_camera() {
    Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.center_on({320.0f, 180.0f});  // 화면 = 맵의 (0,0)~(640,360)
    return camera;
}

TEST(LightDrawing, SkipsOffscreenAndUsesAddBlend) {
    Light near;
    near.position = {100.0f, 100.0f};
    near.radius = 50.0f;
    near.color = {255, 176, 96, 255};
    near.intensity = 0.5f;
    Light edge;  // 중심은 화면 밖이지만 빛이 화면 안으로 들어와요
    edge.position = {-30.0f, 100.0f};
    edge.radius = 50.0f;
    Light far;
    far.position = {2000.0f, 2000.0f};
    far.radius = 50.0f;

    LightingFrame frame;
    const auto stats = wind::engine::queue_lights({near, edge, far}, make_camera(), 0.0, {7}, frame);
    EXPECT_EQ(stats.total, 3u);
    EXPECT_EQ(stats.drawn, 2u);
    EXPECT_EQ(stats.offscreen, 1u);
    ASSERT_EQ(frame.lights.size(), 2u);
    const auto& sprite = frame.lights[0];
    EXPECT_EQ(sprite.blend, wind::engine::BlendMode::Add);
    EXPECT_EQ(sprite.texture.id, 7u);
    EXPECT_EQ(sprite.dest, (wind::engine::RectF{50.0f, 50.0f, 100.0f, 100.0f}));
    EXPECT_EQ(sprite.tint.r, 255);
    EXPECT_EQ(sprite.tint.g, 176);
    EXPECT_EQ(sprite.tint.a, 128);  // 밝기 0.5 -> 진하기 128
}

// 한 화면에 그릴 빛은 최대 64개. 넘는 빛은 건너뛰고 세요.
TEST(LightDrawing, CapsLightsPerFrame) {
    std::vector<Light> lights(70);
    for (auto& light : lights) {
        light.position = {320.0f, 180.0f};
    }
    LightingFrame frame;
    const auto stats = wind::engine::queue_lights(lights, make_camera(), 0.0, {1}, frame);
    EXPECT_EQ(stats.drawn, wind::engine::kMaxLightsPerFrame);
    EXPECT_EQ(stats.over_cap, 70u - wind::engine::kMaxLightsPerFrame);
}

}  // namespace
