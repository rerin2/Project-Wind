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

}  // namespace
