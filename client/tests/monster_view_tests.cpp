// 이 파일은 무엇을 하나요?
// 몬스터 그리기 담당(MonsterView)을 그리기 목록 수준에서 숙제 검사(테스트)해요. (11a)
// - 몬스터마다 그림자(Shadows) 하나와 몸(YSorted) 하나를 넣어요. 몸은 발 위치로 줄 서요
// - 쉬는 몬스터의 그림 속 발 위치가 맵의 발 위치와 같은 화면 점에 와요
// - F4가 켜져 있을 때만 상자(DebugOverlay) 두 개(맞는 상자, 발 상자)를 넣어요
#include "game/presentation/monster_view.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

using wind::engine::DrawLayer;
using wind::engine::RenderQueue;
using wind::game::Monster;
using wind::game::MonsterView;

Monster make_slime() {
    wind::combat::MonsterData data;
    data.sprite = {"monsters/s.bmp", 24, 24, 12, 21};
    return Monster("s", data, {100.0f, 100.0f});
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

TEST(MonsterView, QueuesShadowAndBodyPerMonster) {
    std::vector<Monster> monsters{make_slime(), make_slime()};
    MonsterView view({1}, {{2}, {2}});
    view.update(1.0f / 60.0f, monsters);
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, false, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::Shadows), 2u);
    EXPECT_EQ(count_layer(queue, DrawLayer::YSorted), 2u);
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 0u);  // F4 꺼짐
}

TEST(MonsterView, BodyStandsOnFeet) {
    std::vector<Monster> monsters{make_slime()};
    MonsterView view({1}, {{2}});
    RenderQueue queue;
    const auto camera = make_camera();
    view.queue(camera, monsters, 1.0f, false, queue);  // 시계 0: 숨쉬기 0이라 크기 그대로
    const auto it = std::ranges::find_if(queue.commands(), [](const auto& c) { return c.layer == DrawLayer::YSorted; });
    ASSERT_NE(it, queue.commands().end());
    const auto& draw = it->sprite;
    const auto feet = camera.world_to_screen({100.0f, 100.0f});
    EXPECT_FLOAT_EQ(draw.dest.w, 24.0f);
    EXPECT_FLOAT_EQ(draw.dest.x, std::round(feet.x - 12.0f));  // 그림 속 발(12, 21)이
    EXPECT_FLOAT_EQ(draw.dest.y, std::round(feet.y - 21.0f));  // 맵의 발 위치에 와요
    EXPECT_FLOAT_EQ(it->sort_y, 100.0f);  // 발 y로 줄 서요
}

TEST(MonsterView, DebugBoxesOnlyWithF4) {
    std::vector<Monster> monsters{make_slime()};
    MonsterView view({1}, {{2}});
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, true, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 2u);  // 맞는 상자 + 발 상자
}

}  // namespace
