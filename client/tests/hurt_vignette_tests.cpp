// 이 파일은 무엇을 하나요?
// 피격 붉은 비네트(HurtVignette)를 숙제 검사(테스트)해요. (11c)
// - 맞으면 붉은 가장자리가 화면 전체(640x360)에 그려지고, 0.6초 뒤 사라져요
// - 접근성: 번쩍임 세기 0이면 그리지 않아요
#include "game/presentation/hurt_vignette.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::DrawLayer;
using wind::engine::RenderQueue;
using wind::game::HurtVignette;

TEST(HurtVignette, ShowsRedEdgesThenFades) {
    HurtVignette vignette;
    RenderQueue idle;
    vignette.queue({5}, {640.0f, 360.0f}, idle);
    EXPECT_TRUE(idle.commands().empty());  // 맞기 전에는 없음

    vignette.trigger();
    RenderQueue hit;
    vignette.queue({5}, {640.0f, 360.0f}, hit);
    ASSERT_EQ(hit.commands().size(), 1u);
    const auto& draw = hit.commands().front();
    EXPECT_EQ(draw.layer, DrawLayer::Foreground);  // 조명 뒤: 밤에도 보여요
    EXPECT_FLOAT_EQ(draw.sprite.dest.w, 640.0f);   // 화면 전체
    EXPECT_GT(draw.sprite.tint.r, draw.sprite.tint.g);  // 붉은색
    EXPECT_GT(draw.sprite.tint.a, 150);

    vignette.update(0.3f);
    const float half = vignette.alpha();
    EXPECT_GT(half, 0.0f);
    EXPECT_LT(half, 0.85f * 0.5f);  // 처음엔 빨리 줄어요 (세기 제곱)
    vignette.update(0.31f);
    EXPECT_FLOAT_EQ(vignette.alpha(), 0.0f);  // 0.6초면 사라져요
}

TEST(HurtVignette, AccessibilityZeroHidesIt) {
    HurtVignette vignette;
    vignette.set_strength(0.0f);
    vignette.trigger();
    RenderQueue queue;
    vignette.queue({5}, {640.0f, 360.0f}, queue);
    EXPECT_TRUE(queue.commands().empty());
}

}  // namespace
