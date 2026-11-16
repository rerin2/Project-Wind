// 이 파일은 무엇을 하나요?
// 싸움 화면 담당(CombatView)을 숙제 검사(테스트)해요.
// - 서 있는 허수아비는 그림 0번 칸, 쓰러지면 1번 칸
// - HP가 가득 차 있으면 HP 바를 그리지 않고, 맞으면 그려요
// - F4가 켜져 있을 때만 판정 상자(DebugOverlay)를 넣어요
#include "game/presentation/combat_view.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace {

using wind::engine::DrawLayer;
using wind::engine::RenderQueue;
using wind::game::CombatView;
using wind::game::TrainingDummy;

TrainingDummy make_dummy() {
    wind::combat::TrainingDummyData data;
    data.max_hp = 20;
    data.sprite = {"props/d.bmp", 48, 48, 24, 44};
    return TrainingDummy(1, "d", data, {100.0f, 100.0f});
}

wind::engine::Camera2D make_camera() {
    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    return camera;
}

std::size_t count_layer(const RenderQueue& queue, DrawLayer layer) {
    return static_cast<std::size_t>(std::ranges::count_if(
        queue.commands(), [layer](const auto& c) { return c.layer == layer; }));
}

const wind::engine::DrawCommand* dummy_sprite(const RenderQueue& queue) {
    for (const auto& c : queue.commands()) {
        if (c.layer == DrawLayer::YSorted) return &c;
    }
    return nullptr;
}

TEST(CombatView, FullHpHasNoBarAndStandingFrame) {
    std::vector<TrainingDummy> dummies{make_dummy()};
    CombatView view({1}, {{2}});
    view.update(1.0f / 60.0f, dummies);
    RenderQueue queue;
    view.queue(make_camera(), dummies, std::nullopt, false, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::Foreground), 0u);  // HP 바 없음
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 0u);
    ASSERT_NE(dummy_sprite(queue), nullptr);
    EXPECT_EQ(dummy_sprite(queue)->sprite.source.x, 0);        // 0번 칸
}

TEST(CombatView, HitShowsBarAndDownUsesSecondFrame) {
    std::vector<TrainingDummy> dummies{make_dummy()};
    CombatView view({1}, {{2}});
    dummies[0].take_damage(20);                                 // 쓰러뜨려요
    view.update(1.0f / 60.0f, dummies);
    RenderQueue queue;
    view.queue(make_camera(), dummies, std::nullopt, false, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::Foreground), 4u);  // 테두리·빈칸·꼬리·바
    EXPECT_EQ(dummy_sprite(queue)->sprite.source.x, 48);       // 1번 칸 (쓰러짐)
}

TEST(CombatView, BoxesOnlyWithF4) {
    std::vector<TrainingDummy> dummies{make_dummy()};
    CombatView view({1}, {{2}});
    RenderQueue queue;
    view.queue(make_camera(), dummies, wind::engine::RectF{90.0f, 80.0f, 28.0f, 24.0f}, true, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 2u);  // 허트박스 + 히트박스
}

// ---- 10b: 맞을 때 하얀 번쩍임 ----

// 맞으면 0.08초 동안 같은 그림을 더하기 섞기로 한 번 더 그리고, 그 뒤에는 안 그려요.
TEST(CombatView, FlashDrawsAdditiveCopyBriefly) {
    CombatView view({1}, {{2}});
    const std::vector<TrainingDummy> dummies{make_dummy()};
    view.flash(dummies, 1);
    EXPECT_FLOAT_EQ(view.flash_amount(0), 1.0f);
    RenderQueue queue;
    view.queue(make_camera(), dummies, std::nullopt, false, queue);
    const auto adds = std::ranges::count_if(queue.commands(), [](const auto& c) {
        return c.layer == DrawLayer::YSorted && c.sprite.blend == wind::engine::BlendMode::Add;
    });
    EXPECT_EQ(adds, 1);
    view.update(0.05f, dummies);
    EXPECT_NEAR(view.flash_amount(0), 0.375f, 1e-4f);  // 끝으로 갈수록 흐려져요
    view.update(0.05f, dummies);
    EXPECT_FLOAT_EQ(view.flash_amount(0), 0.0f);
}

// 번쩍임 세기 0이면 (접근성) 맞아도 번쩍이지 않아요. 다른 번호는 안 번쩍여요.
TEST(CombatView, FlashRespectsStrengthAndTarget) {
    CombatView view({1}, {{2}});
    const std::vector<TrainingDummy> dummies{make_dummy()};
    view.flash(dummies, 7);  // 없는 번호
    EXPECT_FLOAT_EQ(view.flash_amount(0), 0.0f);
    view.set_flash_strength(0.0f);
    view.flash(dummies, 1);
    EXPECT_FLOAT_EQ(view.flash_amount(0), 0.0f);
    RenderQueue queue;
    view.queue(make_camera(), dummies, std::nullopt, false, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::YSorted), 1u);  // 원래 그림만
    view.set_flash_strength(0.5f);
    EXPECT_FLOAT_EQ(view.flash_amount(0), 0.5f);
}

}  // namespace
