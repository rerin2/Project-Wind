// 이 파일은 무엇을 하나요?
// 공격 데이터 파일 검사가 맞게 동작하는지 숙제 검사(테스트)를 해요. (명세 9.2)
#include "wind/content/attack_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::parse_attack;

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

json make_doc() {
    return json{{"schema_version", 1},
                {"id", "slash"},
                {"windup", 0.08},
                {"active", 0.10},
                {"recovery", 0.18},
                {"cancel_window", {0.18, 0.36}},
                {"hitbox", {{"offset", {18, -8}}, {"size", {28, 24}}}},
                {"power", 1.0},
                {"knockback", 90},
                {"hitstop", 0.06},
                {"shake", 0.25}};
}

TEST(AttackLoader, ParsesValidAttack) {
    auto doc = make_doc();
    doc["fx"] = "fx\\slash.json";
    doc["next_combo"] = "slash_2";
    doc["mp_cost"] = 3;
    const auto result = parse_attack(doc, "attacks/a.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& a = *result.value;
    EXPECT_EQ(a.id, "slash");
    EXPECT_FLOAT_EQ(a.windup, 0.08f);
    EXPECT_FLOAT_EQ(a.cancel_start, 0.18f);
    EXPECT_FLOAT_EQ(a.cancel_end, 0.36f);
    EXPECT_FLOAT_EQ(a.hitbox.forward, 18.0f);
    EXPECT_FLOAT_EQ(a.hitbox.lift, -8.0f);
    EXPECT_FLOAT_EQ(a.hitbox.length, 28.0f);
    EXPECT_FLOAT_EQ(a.hitbox.width, 24.0f);
    EXPECT_NEAR(a.total_seconds(), 0.36f, 1e-6f);
    EXPECT_EQ(a.fx, "fx/slash.json");  // "\\"는 "/"로 정리
    EXPECT_EQ(a.next_combo, "slash_2");
    EXPECT_EQ(a.mp_cost, 3);
    EXPECT_FALSE(a.sfx.has_value());
}

// 선택 칸이 없으면 기본값 (MP 0, 효과 없음, 다음 콤보 없음)
TEST(AttackLoader, OptionalFieldsDefault) {
    const auto result = parse_attack(make_doc(), "a.json");
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value->mp_cost, 0);
    EXPECT_FALSE(result.value->fx.has_value());
    EXPECT_FALSE(result.value->next_combo.has_value());
}

// 판정 0초, 음수 시간, 범위 밖 숫자는 거절해요.
TEST(AttackLoader, RejectsBadTimes) {
    auto zero_active = make_doc();
    zero_active["active"] = 0;
    EXPECT_TRUE(has_error_at(parse_attack(zero_active, "a.json").errors, "active"));

    auto negative = make_doc();
    negative["windup"] = -0.1;
    EXPECT_TRUE(has_error_at(parse_attack(negative, "a.json").errors, "windup"));

    auto strong = make_doc();
    strong["power"] = 99;
    EXPECT_TRUE(has_error_at(parse_attack(strong, "a.json").errors, "power"));
}

// 취소 구간은 공격이 끝나기 전에 있어야 하고, 시작 <= 끝이어야 해요.
TEST(AttackLoader, CancelWindowMustFitTheAttack) {
    auto late = make_doc();
    late["cancel_window"] = {0.2, 0.5};  // 공격은 0.36초에 끝나요
    EXPECT_TRUE(has_error_at(parse_attack(late, "a.json").errors, "cancel_window"));

    auto backwards = make_doc();
    backwards["cancel_window"] = {0.3, 0.1};
    EXPECT_TRUE(has_error_at(parse_attack(backwards, "a.json").errors, "cancel_window"));
}

// 히트박스는 숫자 두 개씩, 크기는 1 이상이어야 해요.
TEST(AttackLoader, RejectsBadHitbox) {
    auto tiny = make_doc();
    tiny["hitbox"]["size"] = {0, 24};
    EXPECT_TRUE(has_error_at(parse_attack(tiny, "a.json").errors, "hitbox.size"));

    auto three = make_doc();
    three["hitbox"]["offset"] = {1, 2, 3};
    EXPECT_TRUE(has_error_at(parse_attack(three, "a.json").errors, "hitbox.offset"));

    auto missing = make_doc();
    missing.erase("hitbox");
    EXPECT_TRUE(has_error_at(parse_attack(missing, "a.json").errors, "hitbox"));
}

// 콘텐츠 폴더 밖 효과 파일, 음수 MP, 모르는 양식 버전은 거절해요.
TEST(AttackLoader, RejectsUnsafeOrUnknown) {
    auto escape = make_doc();
    escape["sfx"] = "../../secret.wav";
    EXPECT_TRUE(has_error_at(parse_attack(escape, "a.json").errors, "sfx"));

    auto mp = make_doc();
    mp["mp_cost"] = -1;
    EXPECT_TRUE(has_error_at(parse_attack(mp, "a.json").errors, "mp_cost"));

    auto version = make_doc();
    version["schema_version"] = 2;
    EXPECT_TRUE(has_error_at(parse_attack(version, "a.json").errors, "schema_version"));

    EXPECT_FALSE(parse_attack(json::array(), "a.json").ok());
}

}  // namespace
