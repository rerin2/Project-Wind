#pragma once

// 이 파일은 무엇을 하나요?
// UI 테마 파일(content/ui/theme.json)을 읽고 숙제 검사를 해요. (명세 14.4)
// 그림 위치·글꼴 위치는 콘텐츠 폴더 안이어야 하고, 9조각 테두리는 그림 크기의 절반을 넘으면 안 돼요.
// 틀린 곳이 있으면 테마 대신 "어디가 틀렸는지" 오류 목록을 돌려줘요.
// 형식은 docs/ENGINE.md "UI theme" 절에 있어요.

#include "engine/ui/ui_theme.hpp"

#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::engine {

inline constexpr std::int64_t kUiThemeSchemaVersion = 1;

[[nodiscard]] content::LoadResult<UiTheme> parse_ui_theme(const nlohmann::json& document,
                                                          const std::string& file);

[[nodiscard]] content::LoadResult<UiTheme> load_ui_theme_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::engine
