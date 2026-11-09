// 이 파일은 무엇을 하나요?
// 파티클 효과 요리법(JSON) 검사가 맞게 동작하는지 숙제 검사(테스트)를 해요.
// - 올바른 요리법은 통과하고 값이 제대로 들어가나
// - 틀린 칸이 있으면 정확히 어디가 왜 틀렸는지 알려 주나
// - 저장소에 들어 있는 진짜 효과 파일들도 통과하나
#include "engine/fx/particle_effect_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>

namespace {

using nlohmann::json;
using wind::engine::BlendMode;
using wind::engine::ParticleShape;
using wind::engine::parse_particle_effect;

json make_effect() {
    return json{{"schema_version", 1},
                {"id", "petals"},
                {"shape", "pixel"},
                {"blend", "add"},
                {"lifetime", {6, 9}},
                {"speed", 12},
                {"direction", 110},
                {"spread", 30},
                {"start_size", {2, 3}},
                {"start_color", "#f2b8c6"},
                {"end_color", "#d97a9b00"},
                {"wobble", {{"amplitude", 6}, {"frequency", 0.5}}},
                {"rate", 4}};
}

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

// 올바른 요리법: 범위([6, 9])와 숫자 하나(12) 둘 다 받아요. 색의 a가 없으면 255예요.
TEST(ParticleEffectLoader, ParsesValidEffect) {
    const auto result = parse_particle_effect(make_effect(), "fx/petals.json");
    ASSERT_TRUE(result.ok()) << (result.errors.empty() ? "" : wind::content::to_string(result.errors[0]));
    const auto& e = *result.value;
    EXPECT_EQ(e.shape, ParticleShape::Pixel);
    EXPECT_EQ(e.blend, BlendMode::Add);
    EXPECT_FLOAT_EQ(e.lifetime.min, 6.0f);
    EXPECT_FLOAT_EQ(e.lifetime.max, 9.0f);
    EXPECT_FLOAT_EQ(e.speed.min, 12.0f);
    EXPECT_FLOAT_EQ(e.speed.max, 12.0f);
    EXPECT_EQ(e.start_color.a, 255);
    EXPECT_EQ(e.end_color.a, 0);
    EXPECT_EQ(e.end_color.r, 0xd9);
    EXPECT_FLOAT_EQ(e.wobble_frequency, 0.5f);
    EXPECT_FLOAT_EQ(e.rate, 4.0f);
    EXPECT_EQ(e.burst, 0u);
}

// 일부러 여러 군데를 틀리게: 모르는 모양, 거꾸로 된 범위, 이상한 색, 너무 빠른 살랑임
TEST(ParticleEffectLoader, ReportsFieldPaths) {
    auto doc = make_effect();
    doc["shape"] = "star";
    doc["lifetime"] = {9, 6};
    doc["start_color"] = "pink";
    doc["wobble"]["frequency"] = 99;
    doc["spread"] = "wide";
    const auto result = parse_particle_effect(doc, "fx/bad.json");
    ASSERT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "shape"));
    EXPECT_TRUE(has_error_at(result.errors, "lifetime"));
    EXPECT_TRUE(has_error_at(result.errors, "start_color"));
    EXPECT_TRUE(has_error_at(result.errors, "wobble.frequency"));
    EXPECT_TRUE(has_error_at(result.errors, "spread"));
    EXPECT_EQ(result.errors.front().file, "fx/bad.json");
}

// 꼭 있어야 하는 칸이 빠지면 거절해요.
TEST(ParticleEffectLoader, RejectsMissingRequiredField) {
    auto doc = make_effect();
    doc.erase("end_color");
    const auto result = parse_particle_effect(doc, "fx/bad.json");
    ASSERT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "end_color"));
}

// 한 번에 터뜨리지도(burst), 계속 뿜지도(rate) 않으면 아무것도 안 만드니 거절해요.
TEST(ParticleEffectLoader, RejectsEffectThatSpawnsNothing) {
    auto doc = make_effect();
    doc["rate"] = 0;
    const auto result = parse_particle_effect(doc, "fx/bad.json");
    ASSERT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "burst"));
}

// 효과 파일 위치가 콘텐츠 폴더 밖이면 열지 않아요.
TEST(ParticleEffectLoader, RejectsPathOutsideContent) {
    const auto result = wind::engine::load_particle_effect_file(WIND_TEST_CONTENT_DIR, "../secret.json");
    EXPECT_FALSE(result.ok());
}

// 저장소에 들어 있는 진짜 효과 파일들도 검사를 통과해야 해요.
TEST(ParticleEffectLoader, RepositoryEffectsAreValid) {
    for (const char* path : {"fx/dust_walk.json", "fx/dust_dodge.json", "fx/petals.json",
                             "fx/fireflies.json", "fx/torch_embers.json"}) {
        const auto result = wind::engine::load_particle_effect_file(WIND_TEST_CONTENT_DIR, path);
        EXPECT_TRUE(result.ok()) << path << ": "
                                 << (result.errors.empty() ? "" : wind::content::to_string(result.errors[0]));
    }
}

}  // namespace
