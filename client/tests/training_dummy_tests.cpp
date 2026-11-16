// 이 파일은 무엇을 하나요?
// 훈련용 허수아비(TrainingDummy)를 숙제 검사(테스트)해요.
// - 맞으면 HP가 줄고, HP보다 많이는 안 깎여요
// - 0이 되면 쓰러지고, 쓰러진 동안은 안 맞아요
// - 1.5초 뒤 HP가 가득 차서 다시 서요
// - 허트박스는 발 기준으로 놓여요
#include "game/combat/training_dummy.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::TrainingDummy;

wind::combat::TrainingDummyData make_data() {
    wind::combat::TrainingDummyData data;
    data.max_hp = 30;
    data.recover_seconds = 1.5f;
    data.hurtbox = {-10.0f, -34.0f, 20.0f, 34.0f};
    return data;
}

TEST(TrainingDummy, TakesDamage) {
    TrainingDummy dummy(1, "d", make_data(), {100.0f, 200.0f});
    EXPECT_EQ(dummy.hp(), 30);
    EXPECT_EQ(dummy.take_damage(14), 14);
    EXPECT_EQ(dummy.hp(), 16);
    EXPECT_EQ(dummy.take_damage(0), 0);    // 0 피해는 무시
    EXPECT_EQ(dummy.take_damage(-5), 0);   // 음수 피해로 HP가 늘지 않아요
    EXPECT_EQ(dummy.hp(), 16);
}

TEST(TrainingDummy, KnockedDownAtZeroThenRecovers) {
    TrainingDummy dummy(1, "d", make_data(), {100.0f, 200.0f});
    EXPECT_EQ(dummy.take_damage(99), 30);  // HP보다 많이는 안 깎여요
    EXPECT_TRUE(dummy.down());
    EXPECT_FALSE(dummy.can_be_hit());
    EXPECT_EQ(dummy.take_damage(10), 0);   // 쓰러진 동안은 안 맞아요
    dummy.step(1.4f);
    EXPECT_TRUE(dummy.down());
    EXPECT_EQ(dummy.hp(), 0);
    dummy.step(0.11f);                      // 1.5초 넘게 지남 (소수 반올림 오차 때문에 딱 1.5초로 재지 않아요)
    EXPECT_FALSE(dummy.down());
    EXPECT_EQ(dummy.hp(), 30);              // 가득 차서 다시 서요
}

TEST(TrainingDummy, HurtboxFollowsFeet) {
    TrainingDummy dummy(1, "d", make_data(), {100.0f, 200.0f});
    const auto box = dummy.hurtbox();
    EXPECT_FLOAT_EQ(box.x, 90.0f);
    EXPECT_FLOAT_EQ(box.y, 166.0f);
    EXPECT_FLOAT_EQ(box.w, 20.0f);
    EXPECT_FLOAT_EQ(box.h, 34.0f);
}

}  // namespace
