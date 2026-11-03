#pragma once

// 이 파일은 무엇을 하나요?
// 캐릭터 그림 설명서(스프라이트 시트 JSON 파일)를 읽고 검사해요.
// 설명서에 실수가 있으면 "어느 파일 어느 칸이 틀렸어요"라고 알려 줘요.
// 설명서 모양은 docs/ENGINE.md에 적혀 있어요.

#include "engine/animation/animation.hpp"

#include "wind/content/json_reader.hpp"

#include <filesystem>
#include <string_view>

namespace wind::engine {

// 지금 읽을 수 있는 설명서 양식의 버전 번호
inline constexpr std::int64_t kSpriteSheetSchemaVersion = 1;

// 이미 읽어 둔 JSON을 검사해서 SpriteSheet로 만들어요.
// 성공하면 value에 결과가, 실패하면 errors에 틀린 곳 목록이 들어 있어요.
[[nodiscard]] content::LoadResult<SpriteSheet> parse_sprite_sheet(const nlohmann::json& document,
                                                                  const std::string& file);

// 콘텐츠 폴더(게임 자료를 모아 둔 폴더) 안의 설명서 파일을 읽고 검사해요.
[[nodiscard]] content::LoadResult<SpriteSheet> load_sprite_sheet_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::engine
