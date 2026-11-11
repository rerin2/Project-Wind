// 이 파일은 무엇을 하나요?
// 무기 파일 검사와 콤보 순서 검사를 숙제 검사(테스트)해요. (명세 8.4)
#include "wind/content/weapon_loader.hpp"

#include "wind/content/attack_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <map>
#include <string>
#include <string_view>

namespace {

using nlohmann::json;
using wind::content::parse_weapon;

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

json make_attack(std::string_view id) {
    return json{{"schema_version", 1},   {"id", id},       {"windup", 0.08},
                {"active", 0.10},         {"recovery", 0.18}, {"cancel_window", {0.18, 0.36}},
                {"hitbox", {{"offset", {18, -8}}, {"size", {28, 24}}}},
                {"power", 1.0},           {"knockback", 90}, {"hitstop", 0.05},
                {"shake", 0.2}};
}

// 파일 대신 메모리 속 공격 데이터를 돌려주는 가짜 읽기 도우미
struct FakeAttacks {
    std::map<std::string, json, std::less<>> files;

    [[nodiscard]] wind::content::AttackSource source() const {
        return [this](std::string_view path) -> wind::content::LoadResult<wind::combat::AttackData> {
            const auto it = files.find(path);
            if (it == files.end()) {
                return {std::nullopt, {{std::string(path), "", "file not found"}}};
            }
            return wind::content::parse_attack(it->second, std::string(path));
        };
    }
};

FakeAttacks make_attacks() {
    FakeAttacks fake;
    auto a = make_attack("a");
    a["next_combo"] = "b";
    fake.files["attacks/a.json"] = a;
    fake.files["attacks/b.json"] = make_attack("b");
    return fake;
}

json make_doc() {
    return json{{"schema_version", 1},
                {"id", "sword"},
                {"name", "롱소드"},
                {"weight", 6},
                {"base_power", 14},
                {"strength_scaling", 0.8},
                {"stamina_per_attack", 14},
                {"attacks", {"attacks/a.json", "attacks\\b.json"}}};
}

TEST(WeaponLoader, ParsesWeaponAndCombo) {
    const auto fake = make_attacks();
    const auto result = parse_weapon(make_doc(), "weapons/w.json", fake.source());
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& w = *result.value;
    EXPECT_EQ(w.id, "sword");
    EXPECT_EQ(w.name, "롱소드");
    EXPECT_FLOAT_EQ(w.base_power, 14.0f);
    EXPECT_FLOAT_EQ(w.stamina_per_attack, 14.0f);
    EXPECT_FLOAT_EQ(w.crit_bonus, 0.0f);
    ASSERT_EQ(w.combo.size(), 2u);
    EXPECT_EQ(w.combo[0].id, "a");
    EXPECT_EQ(w.combo[1].id, "b");
}

// 공격 파일 하나가 깨졌으면 무기 전체를 거절하고, 그 파일의 오류도 함께 알려 줘요.
TEST(WeaponLoader, BrokenAttackFailsTheWeapon) {
    auto fake = make_attacks();
    fake.files["attacks/b.json"]["active"] = 0;
    const auto result = parse_weapon(make_doc(), "w.json", fake.source());
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "attacks[1]"));
    EXPECT_TRUE(has_error_at(result.errors, "active"));

    auto missing = make_doc();
    missing["attacks"] = {"attacks/a.json", "attacks/nope.json"};
    EXPECT_TRUE(has_error_at(parse_weapon(missing, "w.json", fake.source()).errors, "attacks[1]"));
}

// next_combo가 목록 순서와 다르거나, 마지막 공격에 next_combo가 있으면 거절해요.
TEST(WeaponLoader, ComboLinksMustMatchOrder) {
    auto wrong = make_attacks();
    wrong.files["attacks/a.json"]["next_combo"] = "zzz";
    EXPECT_TRUE(has_error_at(parse_weapon(make_doc(), "w.json", wrong.source()).errors,
                             "attacks[0]"));

    auto dangling = make_attacks();
    dangling.files["attacks/b.json"]["next_combo"] = "a";
    EXPECT_TRUE(has_error_at(parse_weapon(make_doc(), "w.json", dangling.source()).errors,
                             "attacks[1]"));
}

// 같은 공격이 두 번 들어 있거나, 목록이 비었거나, 폴더 밖이면 거절해요.
TEST(WeaponLoader, RejectsBadAttackList) {
    const auto fake = make_attacks();
    auto twice = make_doc();
    twice["attacks"] = {"attacks/b.json", "attacks/b.json"};
    EXPECT_TRUE(has_error_at(parse_weapon(twice, "w.json", fake.source()).errors, "attacks[1]"));

    auto empty = make_doc();
    empty["attacks"] = json::array();
    EXPECT_TRUE(has_error_at(parse_weapon(empty, "w.json", fake.source()).errors, "attacks"));

    auto escape = make_doc();
    escape["attacks"] = {"../attacks/a.json"};
    EXPECT_TRUE(has_error_at(parse_weapon(escape, "w.json", fake.source()).errors, "attacks[0]"));
}

// 숫자 범위 검사와 선택 칸 crit_bonus
TEST(WeaponLoader, ChecksNumbers) {
    const auto fake = make_attacks();
    auto heavy = make_doc();
    heavy["weight"] = -1;
    EXPECT_TRUE(has_error_at(parse_weapon(heavy, "w.json", fake.source()).errors, "weight"));

    auto crit = make_doc();
    crit["crit_bonus"] = 0.1;
    const auto ok = parse_weapon(crit, "w.json", fake.source());
    ASSERT_TRUE(ok.ok());
    EXPECT_FLOAT_EQ(ok.value->crit_bonus, 0.1f);

    crit["crit_bonus"] = 2;
    EXPECT_TRUE(has_error_at(parse_weapon(crit, "w.json", fake.source()).errors, "crit_bonus"));
}

// 저장소에 들어 있는 진짜 롱소드: 3타 콤보, 시작 무기 수치 (명세 8.4)
TEST(WeaponLoader, ShippedLongswordIsValid) {
    const auto result = wind::content::load_weapon_file(WIND_TEST_CONTENT_DIR,
                                                        "weapons/longsword.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    const auto& w = *result.value;
    EXPECT_EQ(w.id, "longsword");
    EXPECT_FLOAT_EQ(w.weight, 6.0f);
    EXPECT_FLOAT_EQ(w.base_power, 14.0f);
    EXPECT_FLOAT_EQ(w.strength_scaling, 0.8f);
    EXPECT_FLOAT_EQ(w.stamina_per_attack, 14.0f);
    ASSERT_EQ(w.combo.size(), 3u);
    EXPECT_EQ(w.combo[0].id, "longsword_1");
    EXPECT_EQ(w.combo[2].id, "longsword_3");
    // 1타는 명세의 "보통" 박자: 0.08 / 0.10 / 0.18
    EXPECT_FLOAT_EQ(w.combo[0].windup, 0.08f);
    EXPECT_FLOAT_EQ(w.combo[0].active, 0.10f);
    EXPECT_FLOAT_EQ(w.combo[0].recovery, 0.18f);
}

}  // namespace
