// 이 파일은 무엇을 하나요?
// 칼자국(SlashArc)을 숙제 검사(테스트)해요.
// - 판정 구간이 시작될 때 한 번 생기고, 판정 + 0.08초 뒤에 사라져요
// - 다음 콤보의 판정이 시작되면 새로 생겨요
// - 그리기 목록에는 빛나는(Add) 그림 하나가 Glow 단계로 들어가요
#include "game/presentation/slash_arc.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

using wind::game::AttackRunner;
using wind::game::SlashArc;

constexpr float kDt = 1.0f / 60.0f;

wind::combat::Weapon make_sword() {
    wind::combat::Weapon w;
    for (int i = 1; i <= 2; ++i) {
        wind::combat::AttackData a;
        a.id = "hit" + std::to_string(i);
        a.windup = 0.05f;
        a.active = 0.10f;
        a.recovery = 0.20f;
        a.cancel_start = 0.15f;
        a.cancel_end = 0.35f;
        w.combo.push_back(a);
    }
    return w;
}

// 판정 중일 때만 히트박스를 주는 작은 도우미
std::optional<wind::engine::RectF> box_of(const AttackRunner& attack) {
    if (attack.phase() != AttackRunner::Phase::Active) return std::nullopt;
    return wind::engine::RectF{100.0f, 100.0f, 28.0f, 24.0f};
}

void tick(SlashArc& arc, AttackRunner& attack, int frames) {
    for (int i = 0; i < frames; ++i) {
        attack.step(kDt);
        arc.update(kDt, attack, box_of(attack), wind::world::Direction::Right);
    }
}

TEST(SlashArc, AppearsWhenActiveStartsAndFades) {
    AttackRunner attack(make_sword());
    SlashArc arc({7});
    EXPECT_FALSE(arc.visible());
    ASSERT_TRUE(attack.start());
    tick(arc, attack, 2);           // 준비 중 (0.05초 전)
    EXPECT_FALSE(arc.visible());
    tick(arc, attack, 2);           // 판정 시작
    EXPECT_TRUE(arc.visible());

    wind::engine::Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    wind::engine::RenderQueue queue;
    arc.queue(camera, queue);
    ASSERT_EQ(queue.commands().size(), 1u);
    EXPECT_EQ(queue.commands()[0].layer, wind::engine::DrawLayer::Glow);
    EXPECT_EQ(queue.commands()[0].sprite.blend, wind::engine::BlendMode::Add);

    tick(arc, attack, 12);          // 0.1 + 0.08초가 지나요
    EXPECT_FALSE(arc.visible());
}

TEST(SlashArc, NextComboMakesANewArc) {
    AttackRunner attack(make_sword());
    SlashArc arc({7});
    ASSERT_TRUE(attack.start());
    tick(arc, attack, 20);          // 1타 판정 끝, 취소 구간 (0.33초)
    EXPECT_FALSE(arc.visible());
    ASSERT_TRUE(attack.start());    // 2타
    tick(arc, attack, 4);
    EXPECT_TRUE(arc.visible());
}

}  // namespace
