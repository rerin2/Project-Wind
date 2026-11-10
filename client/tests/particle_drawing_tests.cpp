// 이 파일은 무엇을 하나요?
// 파티클 점의 색 바뀜, 서서히 나타나기, 살랑임, 그리기 목록 넣기를 숙제 검사(테스트)해요.
#include "engine/fx/particle_drawing.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Particle;

Particle make(float age) {
    Particle p;
    p.lifetime = 2.0f;
    p.age = age;
    p.start_color = {200, 100, 0, 255};
    p.end_color = {0, 100, 200, 0};
    return p;
}

// 수명 절반이면 시작 색과 끝 색의 딱 가운데예요.
TEST(ParticleDrawing, ColorBlendsOverLifetime) {
    const auto c = wind::engine::particle_color(make(1.0f));
    EXPECT_EQ(c.r, 100);
    EXPECT_EQ(c.g, 100);
    EXPECT_EQ(c.b, 100);
    EXPECT_EQ(c.a, 128);
}

// fade_in 0.5: 수명의 처음 절반 동안 0에서 서서히 진해져요. 막 태어났을 땐 안 보여요.
TEST(ParticleDrawing, FadeInStartsInvisible) {
    auto p = make(0.0f);
    p.end_color.a = 255;
    p.fade_in = 0.5f;
    EXPECT_EQ(wind::engine::particle_color(p).a, 0);
    p.age = 0.5f;  // 진행률 0.25 -> fade_in의 절반 -> 반쯤 진함
    EXPECT_NEAR(wind::engine::particle_color(p).a, 127, 1);
    p.age = 1.5f;  // fade_in이 끝났어요
    EXPECT_EQ(wind::engine::particle_color(p).a, 255);
}

// 살랑임 폭이 0이면 안 흔들리고, 있으면 폭 안에서 흔들려요.
TEST(ParticleDrawing, WobbleStaysWithinAmplitude) {
    auto p = make(0.0f);
    EXPECT_FLOAT_EQ(wind::engine::particle_wobble(p), 0.0f);
    p.wobble_amplitude = 6.0f;
    p.wobble_frequency = 1.0f;
    for (int i = 0; i < 40; ++i) {
        p.age = static_cast<float>(i) * 0.05f;
        EXPECT_LE(std::abs(wind::engine::particle_wobble(p)), 6.0f + 1e-4f);
    }
}

// 그리기: 모양에 맞는 그림, 효과의 섞기 방법, 정해 준 단계로 넣어요. 화면 밖 점은 빼요.
TEST(ParticleDrawing, QueuesVisibleParticlesWithShapeAndBlend) {
    wind::engine::ParticlePool pool(4);
    Particle soft = make(0.0f);
    soft.position = {10.0f, 10.0f};
    soft.blend = wind::engine::BlendMode::Add;
    Particle pixel = make(0.0f);
    pixel.position = {20.0f, 20.0f};
    pixel.shape = wind::engine::ParticleShape::Pixel;
    Particle far = make(0.0f);
    far.position = {5000.0f, 5000.0f};
    pool.spawn(soft);
    pool.spawn(pixel);
    pool.spawn(far);

    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.center_on({320.0f, 180.0f});
    wind::engine::RenderQueue queue;
    const auto count = wind::engine::queue_particles(pool, camera, {{1}, {2}},
                                                     wind::engine::DrawLayer::WorldFx, queue);
    ASSERT_EQ(count, 2u);
    const auto& cmds = queue.commands();
    EXPECT_EQ(cmds[0].sprite.texture.id, 1u);
    EXPECT_EQ(cmds[0].sprite.blend, wind::engine::BlendMode::Add);
    EXPECT_EQ(cmds[1].sprite.texture.id, 2u);
    EXPECT_EQ(cmds[1].layer, wind::engine::DrawLayer::WorldFx);
}

// fade_out 0.25: 마지막 25% 동안만 서서히 사라져요. 그 전에는 원래 진하기 그대로.
TEST(ParticleDrawing, FadeOutOnlyAtEnd) {
    auto p = make(0.0f);
    p.start_color.a = 200;
    p.end_color = p.start_color;
    p.fade_out = 0.25f;
    p.age = 1.0f;  // 진행률 0.5
    EXPECT_EQ(wind::engine::particle_color(p).a, 200);
    p.age = 1.75f;  // 진행률 0.875 -> 남은 0.125 / 0.25 = 반
    EXPECT_NEAR(wind::engine::particle_color(p).a, 100, 1);
    p.age = 2.0f;
    EXPECT_EQ(wind::engine::particle_color(p).a, 0);
}

// 빗줄기: 굵기 1, 길이 = 크기, 움직이는 방향으로 돌려요. 아래로 떨어지면 돌림 0도.
TEST(ParticleDrawing, StreakPointsAlongVelocity) {
    wind::engine::ParticlePool pool(2);
    Particle rain = make(0.0f);
    rain.position = {100.0f, 100.0f};
    rain.velocity = {0.0f, 300.0f};
    rain.shape = wind::engine::ParticleShape::Streak;
    rain.start_size = rain.end_size = 10.0f;
    pool.spawn(rain);
    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.center_on({320.0f, 180.0f});
    wind::engine::RenderQueue queue;
    wind::engine::queue_particles(pool, camera, {{1}, {2}}, wind::engine::DrawLayer::WorldFx, queue);
    ASSERT_EQ(queue.commands().size(), 1u);
    const auto& s = queue.commands()[0].sprite;
    EXPECT_EQ(s.texture.id, 2u);  // 픽셀 그림
    EXPECT_FLOAT_EQ(s.dest.w, 1.0f);
    EXPECT_FLOAT_EQ(s.dest.h, 10.0f);
    EXPECT_NEAR(s.rotation_degrees, 0.0f, 1e-3f);
}

// 빛나는 단계를 주면 "더하기" 점만 그 단계로 가요 (조명 뒤에 그려져 밤에도 밝게).
TEST(ParticleDrawing, AdditiveParticlesGoToGlowLayer) {
    wind::engine::ParticlePool pool(2);
    Particle glow = make(0.0f);
    glow.position = {100.0f, 100.0f};
    glow.blend = wind::engine::BlendMode::Add;
    Particle petal = make(0.0f);
    petal.position = {120.0f, 100.0f};
    pool.spawn(glow);
    pool.spawn(petal);
    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.center_on({320.0f, 180.0f});
    wind::engine::RenderQueue queue;
    wind::engine::queue_particles(pool, camera, {{1}, {2}}, wind::engine::DrawLayer::WorldFx, queue,
                                  wind::engine::DrawLayer::Glow);
    ASSERT_EQ(queue.commands().size(), 2u);
    EXPECT_EQ(queue.commands()[0].layer, wind::engine::DrawLayer::Glow);
    EXPECT_EQ(queue.commands()[1].layer, wind::engine::DrawLayer::WorldFx);
}

}  // namespace
