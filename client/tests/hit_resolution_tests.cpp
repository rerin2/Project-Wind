// 이 파일은 무엇을 하나요?
// 칼이 허수아비에 맞았는지 정하는 함수(resolve_swing)를 숙제 검사(테스트)해요.
// - 겹치면 피해 = 공격력 × 위력 - 방어 (최소 1)
// - 안 겹치면 안 맞아요
// - 같은 휘두르기에 같은 대상은 한 번만 맞아요 (9단계 완료 조건)
// - 쓰러진 허수아비는 안 맞아요
#include "game/combat/hit_resolution.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

namespace {

using wind::game::resolve_swing;
using wind::game::Swing;
using wind::game::TargetId;
using wind::game::TrainingDummy;

wind::combat::TrainingDummyData make_data(int hp, float defense = 0.0f) {
    wind::combat::TrainingDummyData data;
    data.max_hp = hp;
    data.defense = defense;
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    return data;
}

// 이미 맞힌 대상을 기억하는 가짜 "한 번만" 장치 (AttackRunner::register_hit와 같은 규칙)
struct HitMemory {
    std::vector<TargetId> seen;
    bool operator()(TargetId id) {
        if (std::ranges::find(seen, id) != seen.end()) return false;
        seen.push_back(id);
        return true;
    }
};

// 허수아비 발 (100, 200): 허트박스는 x 90~110, y 166~200
Swing swing_at(float x, float y, float attack_power, float power, HitMemory& memory) {
    Swing swing;
    swing.hitbox = {x, y, 28.0f, 24.0f};
    swing.attack_power = attack_power;
    swing.power = power;
    swing.register_hit = [&memory](TargetId id) { return memory(id); };
    return swing;
}

TEST(HitResolution, OverlapAppliesDamage) {
    std::vector<TrainingDummy> dummies{{1, "a", make_data(100), {100.0f, 200.0f}}};
    HitMemory memory;
    const auto hits = resolve_swing(swing_at(95.0f, 170.0f, 14.0f, 1.1f, memory), dummies);
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_EQ(hits[0].target, 1u);
    EXPECT_EQ(hits[0].amount, 15);  // 14 × 1.1 = 15.4 -> 버림 15
    EXPECT_EQ(dummies[0].hp(), 85);
    EXPECT_FLOAT_EQ(hits[0].at.x, 100.0f);  // 허트박스 가운데
}

TEST(HitResolution, MissWhenNotOverlapping) {
    std::vector<TrainingDummy> dummies{{1, "a", make_data(100), {100.0f, 200.0f}}};
    HitMemory memory;
    const auto hits = resolve_swing(swing_at(200.0f, 170.0f, 14.0f, 1.0f, memory), dummies);
    EXPECT_TRUE(hits.empty());
    EXPECT_EQ(dummies[0].hp(), 100);
    EXPECT_TRUE(memory.seen.empty());  // 안 닿았으면 "맞힘"으로 기억하지도 않아요
}

// 같은 휘두르기(같은 기억장치)로 여러 박자 판정해도 한 번만 깎여요.
TEST(HitResolution, SameSwingHitsOnce) {
    std::vector<TrainingDummy> dummies{{1, "a", make_data(100), {100.0f, 200.0f}}};
    HitMemory memory;
    for (int step = 0; step < 6; ++step) {
        (void)resolve_swing(swing_at(95.0f, 170.0f, 14.0f, 1.0f, memory), dummies);
    }
    EXPECT_EQ(dummies[0].hp(), 86);
}

// 한 번 휘둘러 두 허수아비에 닿으면 둘 다 맞아요. 방어력이 높아도 최소 1은 깎여요.
TEST(HitResolution, HitsEveryOverlappingDummyWithMinimum) {
    std::vector<TrainingDummy> dummies{{1, "a", make_data(100), {100.0f, 200.0f}},
                                       {2, "b", make_data(100, 500.0f), {115.0f, 200.0f}}};
    HitMemory memory;
    const auto hits = resolve_swing(swing_at(95.0f, 170.0f, 14.0f, 1.0f, memory), dummies);
    ASSERT_EQ(hits.size(), 2u);
    EXPECT_EQ(dummies[1].hp(), 99);  // 방어 500이어도 최소 피해 1
}

TEST(HitResolution, DownDummyIsSkippedAndKnockdownIsReported) {
    std::vector<TrainingDummy> dummies{{1, "a", make_data(10), {100.0f, 200.0f}}};
    HitMemory first;
    const auto hits = resolve_swing(swing_at(95.0f, 170.0f, 14.0f, 1.0f, first), dummies);
    ASSERT_EQ(hits.size(), 1u);
    EXPECT_EQ(hits[0].amount, 10);       // 남은 HP만큼만
    EXPECT_TRUE(hits[0].knocked_down);
    HitMemory second;                    // 새 휘두르기라도
    EXPECT_TRUE(resolve_swing(swing_at(95.0f, 170.0f, 14.0f, 1.0f, second), dummies).empty());  // 쓰러진 동안은 안 맞아요
}

}  // namespace
