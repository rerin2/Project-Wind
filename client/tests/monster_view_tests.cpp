// 이 파일은 무엇을 하나요?
// 몬스터 그리기 담당(MonsterView)을 그리기 목록 수준에서 숙제 검사(테스트)해요. (11a)
// - 몬스터마다 그림자(Shadows) 하나와 몸(YSorted) 하나를 넣어요. 몸은 발 위치로 줄 서요
// - 쉬는 몬스터의 그림 속 발 위치가 맵의 발 위치와 같은 화면 점에 와요
// - F4가 켜져 있을 때만 상자(DebugOverlay) 두 개(맞는 상자, 발 상자)를 넣어요
// - 11b: 알아채면 느낌표, 예고 중에는 바닥에 붉은 경고와 웅크린 그림, 덮치는 동안 F4 빨간 판정 상자
#include "game/presentation/monster_view.hpp"

#include "game/combat/combat_random.hpp"

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
    MonsterView view({1}, {3}, {{2}, {2}});
    view.update(1.0f / 60.0f, monsters);
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, false, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::Shadows), 2u);
    EXPECT_EQ(count_layer(queue, DrawLayer::YSorted), 2u);
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 0u);  // F4 꺼짐
}

TEST(MonsterView, BodyStandsOnFeet) {
    std::vector<Monster> monsters{make_slime()};
    MonsterView view({1}, {3}, {{2}});
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
    MonsterView view({1}, {3}, {{2}});
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, true, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 2u);  // 맞는 상자 + 발 상자
}

// ---------------------------------------------------------------- 11b ----

// 공격이 있는 슬라임 (예고 0.6초, 덮치기 40픽셀)
Monster make_fighter() {
    wind::combat::MonsterData data;
    data.sprite = {"monsters/s.bmp", 24, 24, 12, 21};
    data.notice_seconds = 0.4f;
    wind::combat::AttackData hop;
    hop.windup = 0.6f;
    hop.active = 0.25f;
    hop.recovery = 0.7f;
    hop.lunge = 40.0f;
    hop.hitbox = {6.0f, -6.0f, 18.0f, 14.0f};
    data.attacks.push_back(hop);
    return Monster("s", data, {200.0f, 200.0f});
}

// 주인공이 오른쪽 hero_dx에 서 있을 때 want 상태가 될 때까지 걸어요
void drive_to(Monster& slime, wind::game::MonsterState want, float hero_dx) {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    wind::game::CombatRandom random(1);
    const wind::engine::Vec2 hero{200.0f + hero_dx, 200.0f};
    for (int i = 0; i < 600 && slime.state() != want; ++i) slime.step(1.0f / 60.0f, map, random, hero);
}

TEST(MonsterView, NoticeShowsExclamationMark) {
    std::vector<Monster> monsters{make_fighter()};
    drive_to(monsters[0], wind::game::MonsterState::Notice, 100.0f);
    ASSERT_EQ(monsters[0].state(), wind::game::MonsterState::Notice);
    MonsterView view({1}, {3}, {{2}});
    view.update(1.0f / 60.0f, monsters);
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, false, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::Foreground), 3u);  // 느낌표: 테두리 1 + 노란 속 2 (조명 뒤라 밤에도 보여요)
    EXPECT_EQ(count_layer(queue, DrawLayer::Shadows), 1u);     // 그림자만 (예고 표시는 없어요)
}

TEST(MonsterView, WindupShowsGroundWarningAndCrouchFrame) {
    std::vector<Monster> monsters{make_fighter()};
    drive_to(monsters[0], wind::game::MonsterState::Windup, 30.0f);  // 공격 거리 안
    ASSERT_EQ(monsters[0].state(), wind::game::MonsterState::Windup);
    MonsterView view({1}, {3}, {{2}});
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, false, queue);
    // 바닥: 붉은 경고 2겹(바깥 + 안쪽 심) + 그림자
    EXPECT_EQ(count_layer(queue, DrawLayer::Shadows), 3u);
    const auto warning = std::ranges::find_if(queue.commands(), [](const auto& c) {
        return c.layer == DrawLayer::Shadows && c.sprite.texture.id == 3;  // 빛 그림 = 경고 표시
    });
    ASSERT_NE(warning, queue.commands().end());
    EXPECT_GT(warning->sprite.tint.r, warning->sprite.tint.g);  // 붉은색
    EXPECT_GT(warning->sprite.dest.w, warning->sprite.dest.h);  // 덮칠 방향(오른쪽)으로 길쭉해요
    // 몸은 1번 칸(웅크림)
    const auto body = std::ranges::find_if(queue.commands(), [](const auto& c) { return c.layer == DrawLayer::YSorted; });
    ASSERT_NE(body, queue.commands().end());
    EXPECT_EQ(body->sprite.source.x, 24);
    // 밤에도 보이게 붉은 빛 하나 (쉬는 몬스터는 빛을 내지 않아요)
    const auto lights = view.lights(monsters);
    ASSERT_EQ(lights.size(), 1u);
    EXPECT_GT(lights[0].color.r, lights[0].color.g);
    std::vector<Monster> resting{make_fighter()};
    EXPECT_TRUE(view.lights(resting).empty());
}

TEST(MonsterView, AttackHitboxOnlyWithF4WhileLunging) {
    std::vector<Monster> monsters{make_fighter()};
    drive_to(monsters[0], wind::game::MonsterState::Attack, 30.0f);
    ASSERT_EQ(monsters[0].state(), wind::game::MonsterState::Attack);
    MonsterView view({1}, {3}, {{2}});
    RenderQueue queue;
    view.queue(make_camera(), monsters, 1.0f, true, queue);
    EXPECT_EQ(count_layer(queue, DrawLayer::DebugOverlay), 3u);  // 맞는 상자 + 발 상자 + 공격 판정(빨강)
}

}  // namespace
