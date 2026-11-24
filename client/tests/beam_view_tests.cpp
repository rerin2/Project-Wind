// 이 파일은 무엇을 하나요?
// 혜성 광선 연출(BeamView)의 계산 부분을 숙제 검사(테스트)해요. 그리기는 화면으로 확인해요.
// - 켜질 때 0.08초에 걸쳐 세지고, 막 켜진 화면에서 한 번만 "켜짐"을 알려요 (화면 흔들림)
// - 켜져 있으면 손끝·가운데·끝에 빛 세 개, 꺼지면 0.12초 동안 사라지다 빛이 없어져요
#include "game/presentation/beam_view.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::BeamShape;
using wind::game::BeamView;

BeamShape shining_beam() {
    BeamShape shape;
    shape.active = true;
    shape.hand = {100.0f, 50.0f};
    shape.direction = {1.0f, 0.0f};
    shape.length = 200.0f;
    shape.width = 14.0f;
    shape.light = wind::combat::SpellLight{110.0f, {200, 240, 255, 255}, 1.0f};
    return shape;
}

struct Fixture {
    BeamView view{nullptr, nullptr, {}};  // 효과·그림 없이 (계산만)
    wind::engine::ParticleRandom random{1};
    wind::engine::ParticlePool pool{16};
};

TEST(BeamView, OpensOnceAndGrowsToFullStrength) {
    Fixture f;
    EXPECT_TRUE(f.view.lights().empty());
    EXPECT_TRUE(f.view.update(0.04f, shining_beam(), f.random, f.pool));   // 막 켜짐
    EXPECT_NEAR(f.view.strength(), 0.5f, 1e-4f);                            // 0.08초의 절반
    EXPECT_FALSE(f.view.update(0.04f, shining_beam(), f.random, f.pool));  // 한 번만
    EXPECT_FLOAT_EQ(f.view.strength(), 1.0f);
}

TEST(BeamView, LightsTheHandMiddleAndTip) {
    Fixture f;
    (void)f.view.update(0.1f, shining_beam(), f.random, f.pool);
    const auto lights = f.view.lights();
    ASSERT_EQ(lights.size(), 3u);
    EXPECT_FLOAT_EQ(lights[0].position.x, 100.0f);  // 손끝
    EXPECT_FLOAT_EQ(lights[1].position.x, 200.0f);  // 가운데
    EXPECT_FLOAT_EQ(lights[2].position.x, 300.0f);  // 끝
    EXPECT_GT(lights[1].radius, lights[0].radius);  // 가운데가 조금 더 넓어요
}

TEST(BeamView, FadesOutAfterTheBeamStops) {
    Fixture f;
    (void)f.view.update(0.1f, shining_beam(), f.random, f.pool);
    BeamShape off;
    (void)f.view.update(0.06f, off, f.random, f.pool);
    EXPECT_NEAR(f.view.strength(), 0.5f, 1e-4f);  // 0.12초의 절반
    EXPECT_EQ(f.view.lights().size(), 3u);         // 사라지는 동안은 마지막 자리에서 빛나요
    (void)f.view.update(0.06f, off, f.random, f.pool);
    EXPECT_TRUE(f.view.lights().empty());
}

}  // namespace
