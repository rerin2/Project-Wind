// 이 파일은 무엇을 하나요?
// UI 테마 파일 읽기를 숙제 검사(테스트)해요.
// - 저장소의 진짜 테마(content/ui/theme.json)가 검사를 통과하나
// - 테두리가 너무 두껍거나 글꼴 위치가 콘텐츠 폴더 밖이면 거절하나
#include "engine/ui/ui_theme_loader.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

using nlohmann::json;

json make_theme() {
    return json{{"schema_version", 1},
                {"image", "ui/ui_frames.bmp"},
                {"fonts", {{"regular", {{"file", "fonts/a.ttf"}, {"size", 12}}},
                           {"bold", {{"file", "fonts/b.ttf"}, {"size", 12}}}}},
                {"panel", {{"source", {0, 0, 24, 24}}, {"border", 8}}},
                {"button", {{"source", {24, 0, 24, 16}}, {"border", 5}}},
                {"button_focused", {{"source", {48, 0, 24, 16}}, {"border", 5}}},
                {"colors", {{"text", "#ffffff"}}}};
}

bool has_error_at(const std::vector<wind::content::ContentError>& errors, std::string_view field) {
    return std::ranges::any_of(errors, [field](const auto& e) { return e.field == field; });
}

TEST(UiThemeLoader, ParsesTheme) {
    const auto result = wind::engine::parse_ui_theme(make_theme(), "theme.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_EQ(result.value->panel.border, 8);
    EXPECT_EQ(result.value->button.source.x, 24);
    EXPECT_EQ(result.value->regular.file, "fonts/a.ttf");
    EXPECT_EQ(result.value->text.g, 255);
}

TEST(UiThemeLoader, RejectsBadFramesAndPaths) {
    auto doc = make_theme();
    doc["panel"]["border"] = 13;                // 24의 절반(12)보다 두꺼움
    doc["fonts"]["bold"]["file"] = "../x.ttf";  // 콘텐츠 폴더 밖
    doc["button"]["source"] = {0, 0, 0, 16};    // 폭 0
    const auto result = wind::engine::parse_ui_theme(doc, "theme.json");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(has_error_at(result.errors, "panel.border"));
    EXPECT_TRUE(has_error_at(result.errors, "fonts.bold.file"));
    EXPECT_TRUE(has_error_at(result.errors, "button.source[2]"));
}

TEST(UiThemeLoader, ShippedThemeIsValid) {
    const auto result = wind::engine::load_ui_theme_file(WIND_TEST_CONTENT_DIR, "ui/theme.json");
    ASSERT_TRUE(result.ok()) << wind::content::to_string(result.errors.front());
    EXPECT_FALSE(result.value->regular.file.empty());
}

}  // namespace
