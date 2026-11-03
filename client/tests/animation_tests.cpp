// 이 파일은 무엇을 하나요?
// 움직이는 그림(애니메이션)이 맞게 넘어가는지, 캐릭터 그림 설명서(JSON) 검사가 맞는지
// 숙제 검사(테스트)를 해요.
// - 시간이 흐르면 다음 그림으로 넘어가나, 끝나면 처음으로 돌아가나(반복), 멈추나
// - 설명서에 실수가 있으면 정확히 어디가 틀렸는지 알려 주나
#include "engine/animation/animation.hpp"
#include "engine/animation/sprite_sheet_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

using nlohmann::json;
using wind::engine::AnimationClip;
using wind::engine::Animator;

// 검사용 클립: 그림 판의 4, 5, 6번 칸을 0.1초씩 보여 줘요.
AnimationClip make_clip(bool loop) {
    return {{4, 5, 6}, {0.1, 0.1, 0.1}, loop};
}

// 0.05초 뒤엔 아직 첫 그림(4), 0.1초가 넘으면 둘째 그림(5), 0.2초가 넘으면 셋째 그림(6)
TEST(Animator, AdvancesFramesOverTime) {
    const auto clip = make_clip(true);
    Animator animator;
    animator.play(&clip);
    EXPECT_EQ(animator.column(), 4);
    animator.update(0.05);
    EXPECT_EQ(animator.column(), 4);
    animator.update(0.06);
    EXPECT_EQ(animator.column(), 5);
    animator.update(0.1);
    EXPECT_EQ(animator.column(), 6);
}

// 반복 클립은 세 장(0.3초)을 다 보여 주면 다시 첫 그림으로 돌아가야 해요.
TEST(Animator, LoopsBackToFirstFrame) {
    const auto clip = make_clip(true);
    Animator animator;
    animator.play(&clip);
    animator.update(0.31);
    EXPECT_EQ(animator.column(), 4);
    EXPECT_FALSE(animator.finished());
}

// 반복하지 않는 클립은 마지막 그림에서 멈추고 "끝났어요"(finished)가 돼야 해요.
TEST(Animator, NonLoopingClipStopsOnLastFrame) {
    const auto clip = make_clip(false);
    Animator animator;
    animator.play(&clip);
    animator.update(5.0);
    EXPECT_EQ(animator.column(), 6);
    EXPECT_TRUE(animator.finished());
}

// 같은 클립을 또 틀어도 하던 곳부터 계속해야 해요. "처음부터"(restart)를 부탁하면 처음으로.
TEST(Animator, ReplayingSameClipKeepsPosition) {
    const auto clip = make_clip(true);
    Animator animator;
    animator.play(&clip);
    animator.update(0.15);
    animator.play(&clip);
    EXPECT_EQ(animator.column(), 5);
    animator.play(&clip, /*restart=*/true);
    EXPECT_EQ(animator.column(), 4);
}

// 검사용 캐릭터 그림 설명서: 32x48 그림, 네 방향, "idle"(서 있기)와 "walk"(걷기)
json make_sheet() {
    return json{{"schema_version", 1},
                {"id", "hero"},
                {"image", "sprites/hero.bmp"},
                {"frame_width", 32},
                {"frame_height", 48},
                {"origin", {{"x", 16}, {"y", 46}}},
                {"direction_rows", {{"down", 0}, {"left", 1}, {"right", 2}, {"up", 3}}},
                {"clips",
                 {{"idle", {{"frames", {0}}, {"frame_ms", {500}}, {"loop", true}}},
                  {"walk", {{"frames", {0, 1, 2, 3}}, {"frame_ms", {120, 120, 120, 120}}, {"loop", true}}}}}};
}

// 오류 목록에 "이 칸이 틀렸어요"(field)가 들어 있는지 찾아 주는 도우미
bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 올바른 설명서는 통과해야 하고, 왼쪽(2번째 줄)의 2번 칸은 그림 판의 (64, 48) 위치여야 해요.
TEST(SpriteSheetLoader, ParsesValidSheet) {
    const auto result = wind::engine::parse_sprite_sheet(make_sheet(), "hero.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& sheet = *result.value;
    ASSERT_NE(sheet.clip("walk"), nullptr);
    EXPECT_EQ(sheet.clip("walk")->frames.size(), 4u);
    // 왼쪽(행 1)의 열 2 -> (64, 48)
    EXPECT_EQ(sheet.frame_rect(wind::world::Direction::Left, 2), (wind::engine::RectI{64, 48, 32, 48}));
}

// 일부러 실수 네 개를 넣은 설명서: 네 군데 모두 정확한 위치로 오류가 나와야 해요.
TEST(SpriteSheetLoader, RejectsBadClipsAndMissingDirection) {
    auto doc = make_sheet();
    doc["clips"]["walk"]["frame_ms"] = {120, 0, 120, 120};   // 0ms 프레임 금지
    doc["clips"]["idle"]["frames"] = {0, 1};                 // frame_ms와 길이 불일치
    doc["direction_rows"].erase("up");
    doc["origin"]["y"] = 99;                                  // 프레임 밖
    const auto result = wind::engine::parse_sprite_sheet(doc, "hero.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "clips.walk.frame_ms[1]"));
    EXPECT_TRUE(has_error_at(result.errors, "clips.idle.frame_ms"));
    EXPECT_TRUE(has_error_at(result.errors, "direction_rows.up"));
    EXPECT_TRUE(has_error_at(result.errors, "origin.y"));
}

// 저장소에 들어 있는 진짜 캐릭터 설명서도 검사를 통과해야 해요.
// (누군가 설명서를 잘못 고치면 이 검사가 바로 알려 줘요)
TEST(SpriteSheetLoader, RepositoryHeroSheetIsValid) {
    const auto result = wind::engine::load_sprite_sheet_file(WIND_TEST_CONTENT_DIR, "sprites/test_hero.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_NE(result.value->clip("idle"), nullptr);
    EXPECT_NE(result.value->clip("walk"), nullptr);
}

}  // namespace
