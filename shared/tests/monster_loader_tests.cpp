// 이 파일은 무엇을 하나요?
// 몬스터 설명 파일 읽기를 숙제 검사(테스트)해요. (11a)
// - 저장소의 진짜 슬라임 파일이 검사를 통과하나
// - 범위 밖 숫자, 폴더 밖 그림, 거리 순서가 틀린 파일, 쉬는 시간 [최소, 최대]가 뒤집힌 파일은 거절하나
// - 공격 목록(11b): 비었거나, 못 읽는 공격 파일이 있으면 거절하나
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
                {"rune_reward", 12},
                {"move_speed", 40},
                {"detect_range", 120},
                {"attack_range", 28},
                {"leash_range", 260},
                {"notice_seconds", 0.4},
                {"wander", {{"radius", 40}, {"speed", 18}, {"pause", {1.0, 2.5}}}},
                {"body", {12, 6}},
                {"hurtbox", {{"offset", {-9, -13}}, {"size", {18, 13}}}},
                {"sprite", {{"image", "monsters/s.bmp"}, {"frame", {24, 24}}, {"origin", {12, 21}}}},
                {"attacks", {"attacks/hop.json"}},
                {"stagger_seconds", 0.25}};
}

// 오류 목록에 이 칸 주소의 오류가 있나요?
bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 가짜 공격 파일 읽기: "attacks/hop.json"만 있고, 다른 위치는 "파일 없음"이에요.
wind::content::LoadResult<wind::combat::AttackData> fake_attack(std::string_view path) {
    if (path != "attacks/hop.json") {
        wind::content::ErrorList errors;
        errors.add(std::string(path), "", "file not found");
        return {std::nullopt, errors.take()};
    }
    wind::combat::AttackData attack;
    attack.id = "hop";
    attack.active = 0.25f;
    attack.lunge = 40.0f;
    return {attack, {}};
}

// 가짜 공격 읽기로 몬스터 설명을 검사해요
wind::content::LoadResult<wind::combat::MonsterData> parse(const json& doc) {
    return wind::content::parse_monster(doc, "m.json", fake_attack);
}

TEST(MonsterLoader, ParsesMonster) {
    const auto result = parse(make_monster());
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_FLOAT_EQ(result.value->notice_seconds, 0.4f);
    ASSERT_EQ(result.value->attacks.size(), 1u);         // 공격 목록 (11b)
    EXPECT_FLOAT_EQ(result.value->attacks[0].lunge, 40.0f);
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
    const auto result = parse(doc);
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
    const auto result = parse(doc);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "detect_range"));

    auto attack_doc = make_monster();
    attack_doc["attack_range"] = 150;  // 감지 120보다 커요
    const auto attack_result = parse(attack_doc);
    EXPECT_FALSE(attack_result.ok());
    EXPECT_TRUE(has_error_at(attack_result.errors, "attack_range"));

    auto wander_doc = make_monster();
    wander_doc["wander"]["radius"] = 150;  // 150 + 감지 120 = 270 > 추적 한계 260: 알아채자마자 포기할 수 있어요
    const auto wander_result = parse(wander_doc);
    EXPECT_FALSE(wander_result.ok());
    EXPECT_TRUE(has_error_at(wander_result.errors, "wander.radius"));
}

TEST(MonsterLoader, RejectsBadAttackList) {
    auto empty = make_monster();
    empty["attacks"] = json::array();  // 공격이 하나도 없으면 덤빌 수 없어요
    const auto empty_result = parse(empty);
    EXPECT_FALSE(empty_result.ok());
    EXPECT_TRUE(has_error_at(empty_result.errors, "attacks"));

    auto missing = make_monster();
    missing["attacks"] = {"attacks/hop.json", "attacks/nope.json"};  // 두 번째 파일이 없어요
    const auto missing_result = parse(missing);
    EXPECT_FALSE(missing_result.ok());
    EXPECT_TRUE(has_error_at(missing_result.errors, "attacks[1]"));

    auto escape = make_monster();
    escape["attacks"] = {"../evil.json"};  // 콘텐츠 폴더 밖
    EXPECT_FALSE(parse(escape).ok());
}

// 11d: 움찔 시간은 0~2초, 사라질 때 효과(death_fx)는 없어도 되고 있으면 콘텐츠 폴더 안이어야 해요
TEST(MonsterLoader, ReadsStaggerAndOptionalDeathFx) {
    const auto plain = parse(make_monster());
    ASSERT_TRUE(plain.ok());
    EXPECT_FLOAT_EQ(plain.value->stagger_seconds, 0.25f);
    EXPECT_FALSE(plain.value->death_fx.has_value());  // 없으면 효과 없이 사라져요

    auto with_fx = make_monster();
    with_fx["death_fx"] = "fx\\slime_burst.json";
    const auto fx = parse(with_fx);
    ASSERT_TRUE(fx.ok());
    EXPECT_EQ(*fx.value->death_fx, "fx/slime_burst.json");  // "\"는 "/"로 정리돼요

    auto bad = make_monster();
    bad["death_fx"] = "../evil.json";  // 콘텐츠 폴더 밖
    bad["stagger_seconds"] = 5;        // 너무 길어요
    const auto result = parse(bad);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "death_fx"));
    EXPECT_TRUE(has_error_at(result.errors, "stagger_seconds"));
}

TEST(MonsterLoader, RejectsReversedPause) {
    auto doc = make_monster();
    doc["wander"]["pause"] = {3.0, 1.0};  // 최소가 최대보다 커요
    const auto result = parse(doc);
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "wander.pause"));
}

TEST(MonsterLoader, RejectsMissingFieldsAndWrongVersion) {
    auto doc = make_monster();
    doc.erase("leash_range");  // 꼭 있어야 하는 칸
    EXPECT_FALSE(parse(doc).ok());

    auto old = make_monster();
    old["schema_version"] = 2;  // 모르는 양식 버전
    EXPECT_FALSE(parse(old).ok());

    EXPECT_FALSE(parse(json::array()).ok());  // { } 묶음이 아님
}

TEST(MonsterLoader, PathOutsideContentIsRejected) {
    const auto result = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "../secret.json");
    EXPECT_FALSE(result.ok());
}

TEST(MonsterLoader, ShippedSlimeIsValid) {
    const auto result = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->id, "slime_green");
    ASSERT_EQ(result.value->attacks.size(), 1u);  // 진짜 공격 파일(slime_hop)도 함께 읽어요
    EXPECT_EQ(result.value->attacks[0].id, "slime_hop");
}

}  // namespace

// 13c: 저장소의 새 몬스터 세 종류가 열리고, keep_away는 공격 거리보다 짧아야 해요
TEST(MonsterLoader, RepositoryMonstersLoad) {
    for (const char* path : {"monsters/slime_green.json", "monsters/bat.json", "monsters/mushroom.json",
                             "monsters/frog.json"}) {
        const auto result = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, path);
        ASSERT_TRUE(result.ok()) << path << ": " << wind::content::to_string(result.errors.front());
    }
    const auto frog = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/frog.json");
    EXPECT_FLOAT_EQ(frog.value->keep_away, 70.0f);
    EXPECT_TRUE(frog.value->attacks.front().projectile.has_value());  // 침
}

TEST(MonsterLoader, KeepAwayMustBeShorterThanAttackRange) {
    auto doc = make_monster();
    doc["keep_away"] = 28;  // 공격 거리 28과 같으면 늘 물러나기만 해요
    EXPECT_TRUE(has_error_at(parse(doc).errors, "keep_away"));
    doc["keep_away"] = 10;
    EXPECT_TRUE(parse(doc).ok());
}
