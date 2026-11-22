// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 연출 검사 (13b)
// - 찾은 쿠션은 피어난 모습으로 시작하고, 못 찾은 쿠션은 bloom으로 피어나며 꽃잎이 터지고 빛이 번쩍해요
// - 쉬기 시작하면 하트 두 개, 웅크려 졸면 z가 떠오르고, 쉬는 동안 초록 빛이 켜져요
// - 그리기 목록에 쿠션과 떠오르는 것들이 들어가요
#include "game/presentation/cushion_view.hpp"

#include "engine/fx/particle_effect.hpp"

#include <gtest/gtest.h>

using wind::game::Cushion;
using wind::game::CushionView;
using wind::game::HeroRest;

namespace {

constexpr float kDt = 1.0f / 60.0f;

std::vector<Cushion> two() {
    return {Cushion{"found", "찾은", {3, 3}, {112.0f, 128.0f}}, Cushion{"new", "새", {8, 3}, {272.0f, 128.0f}}};
}

CushionView make_view(const wind::engine::ParticleEffect* fx = nullptr) {
    return CushionView(two(), [](std::string_view id) { return id == "found"; }, {}, fx);
}

wind::engine::ParticleEffect burst_effect() {
    wind::engine::ParticleEffect fx;
    fx.burst = 10;
    fx.lifetime = {1.0f, 1.0f};
    return fx;
}

}  // namespace

TEST(CushionView, StartsBloomedOnlyWhereDiscovered) {
    const auto view = make_view();
    EXPECT_TRUE(view.bloomed(0));
    EXPECT_FALSE(view.bloomed(1));
}

TEST(CushionView, BloomPopsWithParticlesAndLight) {
    const auto fx = burst_effect();
    auto view = make_view(&fx);
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(100);
    view.bloom("new");
    EXPECT_TRUE(view.bloomed(1));
    view.update(kDt, nullptr, HeroRest{}, random, pool);
    EXPECT_EQ(pool.size(), 10u);       // 꽃잎이 한 번 터져요
    EXPECT_EQ(view.lights().size(), 1u);  // 연보라 빛이 번쩍
    for (int i = 0; i < 60; ++i) view.update(kDt, nullptr, HeroRest{}, random, pool);
    EXPECT_TRUE(view.lights().empty());   // 0.8초 뒤 사그라들어요
    view.bloom("new");                    // 이미 피었으면 아무 일 없음
    view.update(kDt, nullptr, HeroRest{}, random, pool);
    EXPECT_TRUE(view.lights().empty());
}

TEST(CushionView, RestingShowsHeartsThenZsAndGreenLight) {
    auto view = make_view();
    const auto cushions = two();
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(10);
    HeroRest rest;
    rest.sit_now(cushions[0].seat);
    view.update(kDt, &cushions[0], rest, random, pool);
    EXPECT_EQ(view.floater_count(), 2u);          // 하트 두 개
    ASSERT_EQ(view.lights().size(), 1u);          // 초록 빛
    EXPECT_GT(view.lights()[0].color.g, view.lights()[0].color.r);
    for (int i = 0; i < 150; ++i) {               // 2.5초: 하트는 사라지고, 졸기 시작해 z가 떠요
        (void)rest.step(kDt, cushions[0].seat);
        view.update(kDt, &cushions[0], rest, random, pool);
    }
    EXPECT_GE(view.floater_count(), 1u);
    view.update(kDt, nullptr, HeroRest{}, random, pool);  // 일어나서 쿠션을 떠나면
    EXPECT_TRUE(view.lights().empty());                    // 빛도 꺼져요
}

TEST(CushionView, QueuesCushionsAndFloaters) {
    auto view = make_view();
    const auto cushions = two();
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(10);
    HeroRest rest;
    rest.sit_now(cushions[0].seat);
    view.update(kDt, &cushions[0], rest, random, pool);
    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    wind::engine::RenderQueue queue;
    view.queue(camera, queue);
    EXPECT_EQ(queue.commands().size(), 4u);  // 쿠션 2 + 하트 2
}
