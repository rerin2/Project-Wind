// 이 파일은 무엇을 하나요?
// 몬스터 설명 파일 읽기를 숙제 검사(테스트)해요. (11a)
// - 저장소의 진짜 슬라임 파일이 검사를 통과하나
// - 범위 밖 숫자, 폴더 밖 그림, 거리 순서가 틀린 파일, 쉬는 시간 [최소, 최대]가 뒤집힌 파일은 거절하나
#include "wind/content/monster_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;

// 통과하는 몬스터 설명 (테스트마다 한 칸씩 망가뜨려요)
json make_monster() {
    return json{{"schema_version", 1},
                {"id", "slime"},
                {"name", "슬라임"},
                {"max_hp", 30},
                {"attack_power", 4},
                {"defense", 0},
                {"move_speed", 40},
                {"detect_range", 120},
                {"attack_range", 28},
                {"leash_range", 260},
                {"wander", {{"radius", 40}, {"speed", 18}, {"pause", {1.0, 2.5}}}},
                {"body", {12, 6}},
                {"hurtbox", {{"offset", {-9, -13}}, {"size", {18, 13}}}},
                {"sprite", {{"image", "monsters/s.bmp"}, {"frame", {24, 24}}, {"origin", {12, 21}}}}};
}

// 오류 목록에 이 칸 주소의 오류가 있나요?
bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

TEST(MonsterLoader, ParsesMonster) {
    const auto result = wind::content::parse_monster(make_monster(), "m.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->max_hp, 30);
    EXPECT_FLOAT_EQ(result.value->detect_range, 120.0f);
    EXPECT_FLOAT_EQ(result.value->wander.pause_max, 2.5f);
    EXPECT_FLOAT_EQ(result.value->body_width, 12.0f);
    EXPECT_FLOAT_EQ(result.value->hurtbox.h, 13.0f);
    EXPECT_EQ(result.value->sprite.origin_y, 21);
}

TEST(MonsterLoader, RejectsBadValues) {
    auto doc = make_monster();
    doc["max_hp"] = 0;                        // HP는 1 이상
    doc["move_speed"] = 0;                    // 0이면 못 움직여요
    doc["defense"] = -1;                      // 방어력은 0 이상
    doc["body"] = {0, 6};                     // 발 상자 크기 0
    doc["sprite"]["image"] = "../evil.bmp";   // 콘텐츠 폴더 밖
    doc["wander"]["speed"] = 1000;            // 너무 빨라요
    const auto result = wind::content::parse_monster(doc, "m.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "max_hp"));
    EXPECT_TRUE(has_error_at(result.errors, "move_speed"));
    EXPECT_TRUE(has_error_at(result.errors, "defense"));
    EXPECT_TRUE(has_error_at(result.errors, "body"));
    EXPECT_TRUE(has_error_at(result.errors, "sprite.image"));
    EXPECT_TRUE(has_error_at(result.errors, "wander.speed"));
}

TEST(MonsterLoader, RejectsRangesOutOfOrder) {
    auto doc = make_monster();
    doc["attack_range"] = 150;  // 감지(120)보다 멀리서 공격하면 알아채기 전에 덤벼요
    doc["detect_range"] = 300;  // 추적 한계(260)보다 멀리서 알아채면 알아채자마자 포기해요
    const auto result = wind::content::parse_monster(doc, "m.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "detect_range"));

    auto attack_doc = make_monster();
    attack_doc["attack_range"] = 150;  // 감지 120보다 커요
    const auto attack_result = wind::content::parse_monster(attack_doc, "m.json");
    EXPECT_FALSE(attack_result.ok());
    EXPECT_TRUE(has_error_at(attack_result.errors, "attack_range"));

    auto wander_doc = make_monster();
    wander_doc["wander"]["radius"] = 300;  // 추적 한계 260보다 멀리 배회하면 안 돼요
    const auto wander_result = wind::content::parse_monster(wander_doc, "m.json");
    EXPECT_FALSE(wander_result.ok());
    EXPECT_TRUE(has_error_at(wander_result.errors, "wander.radius"));
}

TEST(MonsterLoader, RejectsReversedPause) {
    auto doc = make_monster();
    doc["wander"]["pause"] = {3.0, 1.0};  // 최소가 최대보다 커요
    const auto result = wind::content::parse_monster(doc, "m.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "wander.pause"));
}

TEST(MonsterLoader, RejectsMissingFieldsAndWrongVersion) {
    auto doc = make_monster();
    doc.erase("leash_range");  // 꼭 있어야 하는 칸
    EXPECT_FALSE(wind::content::parse_monster(doc, "m.json").ok());

    auto old = make_monster();
    old["schema_version"] = 2;  // 모르는 양식 버전
    EXPECT_FALSE(wind::content::parse_monster(old, "m.json").ok());

    EXPECT_FALSE(wind::content::parse_monster(json::array(), "m.json").ok());  // { } 묶음이 아님
}

TEST(MonsterLoader, PathOutsideContentIsRejected) {
    const auto result = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "../secret.json");
    EXPECT_FALSE(result.ok());
}

TEST(MonsterLoader, ShippedSlimeIsValid) {
    const auto result = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->id, "slime_green");
}

}  // namespace
