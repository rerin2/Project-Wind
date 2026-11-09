// 이 파일은 무엇을 하나요?
// 필드 조명 담당(FieldLighting)을 숙제 검사(테스트)해요.
// - 주변광이 없는 맵은 조명을 끄나
// - 칸 단위 빛 위치를 픽셀로 바꾸나, 주인공 빛이 주인공을 따라가나
// - 빛 자리 효과(횃불 불티)를 뿜나
#include "game/presentation/field_lighting.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::FieldLighting;

wind::engine::Camera2D make_camera() {
    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.center_on({320.0f, 180.0f});
    return camera;
}

// 밝은 낮 맵(주변광 없음): 빛이 있어도 조명을 쓰지 않아요.
TEST(FieldLighting, DisabledWithoutAmbientLight) {
    wind::world::Ambience ambience;
    ambience.lights.push_back({});
    FieldLighting lighting(ambience, 32, {}, 0, {});
    EXPECT_FALSE(lighting.enabled());
    EXPECT_FALSE(lighting.build(make_camera(), {1}).enabled);
}

TEST(FieldLighting, BuildsLightsFromAmbience) {
    wind::world::Ambience ambience;
    ambience.ambient_light = wind::world::Rgba{28, 26, 48, 255};
    wind::world::LightDef torch;
    torch.x = 3.5f;  // 칸 단위
    torch.y = 2.0f;
    torch.radius = 100.0f;
    torch.fx = "fx/embers.json";
    ambience.lights.push_back(torch);
    ambience.player_light = wind::world::PlayerLight{60.0f, {255, 226, 184, 255}, 0.5f};

    wind::engine::ParticleEffect embers;
    embers.lifetime = {1.0f, 1.0f};
    embers.rate = 60.0f;
    const std::map<std::string, wind::engine::ParticleEffect> fx{{"fx/embers.json", embers}};
    FieldLighting lighting(ambience, 32, fx, 0, {});

    ASSERT_TRUE(lighting.enabled());
    ASSERT_EQ(lighting.lights().size(), 1u);
    EXPECT_EQ(lighting.lights()[0].position, (wind::engine::Vec2{112.0f, 64.0f}));  // 3.5×32, 2×32

    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(100);
    for (int i = 0; i < 30; ++i) {  // 화면 30장 = 0.5초
        lighting.update(1.0f / 60.0f, {200.0f, 150.0f}, random, pool);
    }
    EXPECT_GE(pool.size(), 29u);  // 1초에 60개 × 0.5초 = 30개 불티
    EXPECT_LE(pool.size(), 30u);

    const auto& frame = lighting.build(make_camera(), {9});
    EXPECT_TRUE(frame.enabled);
    EXPECT_EQ(frame.ambient.b, 48);
    ASSERT_EQ(frame.lights.size(), 2u);  // 주인공 빛 + 횃불
    // 주인공 빛이 맨 앞이고, 가슴 높이(발에서 14픽셀 위)에 있어요: 중심 (200, 136), 반지름 60
    EXPECT_EQ(frame.lights[0].dest, (wind::engine::RectF{140.0f, 76.0f, 120.0f, 120.0f}));
}

// 성능 시험용 빛은 정해 준 개수만큼 더해져요.
TEST(FieldLighting, AddsStressLights) {
    wind::world::Ambience ambience;
    ambience.ambient_light = wind::world::Rgba{0, 0, 0, 255};
    FieldLighting lighting(ambience, 32, {}, 32, {320.0f, 180.0f});
    EXPECT_EQ(lighting.lights().size(), 32u);
}

}  // namespace
