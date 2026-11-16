// 이 파일은 무엇을 하나요?
// 허수아비 설명 파일 읽기를 숙제 검사(테스트)해요.
// - 저장소의 진짜 파일이 검사를 통과하나
// - HP 0, 음수 방어력, 칸 밖 발 위치, 폴더 밖 그림은 거절하나
#include "wind/content/training_dummy_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;

json make_dummy() {
    return json{{"schema_version", 1},
                {"id", "training_dummy"},
                {"name", "허수아비"},
                {"max_hp", 100},
                {"defense", 0},
                {"recover_seconds", 1.5},
                {"hurtbox", {{"offset", {-10, -34}}, {"size", {20, 34}}}},
                {"sprite", {{"image", "props/d.bmp"}, {"frame", {32, 48}}, {"origin", {16, 44}}}}};
}

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

TEST(TrainingDummyLoader, ParsesDummy) {
    const auto result = wind::content::parse_training_dummy(make_dummy(), "d.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->max_hp, 100);
    EXPECT_FLOAT_EQ(result.value->hurtbox.y, -34.0f);
    EXPECT_FLOAT_EQ(result.value->hurtbox.h, 34.0f);
    EXPECT_EQ(result.value->sprite.origin_y, 44);
}

TEST(TrainingDummyLoader, RejectsBadValues) {
    auto doc = make_dummy();
    doc["max_hp"] = 0;                                  // HP는 1 이상
    doc["defense"] = -5;                                // 방어력은 0 이상
    doc["sprite"]["origin"] = {40, 44};                 // 발 위치가 32칸 밖
    doc["sprite"]["image"] = "../evil.bmp";             // 콘텐츠 폴더 밖
    doc["hurtbox"]["size"] = {0, 34};                   // 크기 0
    const auto result = wind::content::parse_training_dummy(doc, "d.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "max_hp"));
    EXPECT_TRUE(has_error_at(result.errors, "defense"));
    EXPECT_TRUE(has_error_at(result.errors, "sprite.image"));
    EXPECT_TRUE(has_error_at(result.errors, "hurtbox.size[0]"));
}

TEST(TrainingDummyLoader, OriginOutsideFrameIsRejected) {
    auto doc = make_dummy();
    doc["sprite"]["origin"] = {40, 44};
    const auto result = wind::content::parse_training_dummy(doc, "d.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "sprite.origin"));
}

TEST(TrainingDummyLoader, ShippedDummyIsValid) {
    const auto result =
        wind::content::load_training_dummy_file(WIND_TEST_CONTENT_DIR, "props/training_dummy.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->name, "훈련용 허수아비");
}

}  // namespace
