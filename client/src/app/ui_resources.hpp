#pragma once

// 이 파일은 무엇을 하나요?
// UI 준비물(테마, 글꼴, 테두리 그림)을 한 번에 불러오는 도우미예요.
// 테마 파일이 잘못돼도 게임은 멈추지 않아요: 기본 테마로 계속하고, 이유를 기록해요.
// 글꼴이 없으면 글만 안 보이고, 테두리 그림이 없으면 단색 네모로 대신 그려요.

#include "engine/render/text_renderer.hpp"
#include "engine/resources/resource_manager.hpp"
#include "engine/ui/ui_theme.hpp"

#include <string_view>

namespace wind::client {

struct UiResources {
    engine::UiTheme theme;
    engine::TextureHandle frames;  // 테두리 그림 판 (없으면 빈 번호표)
};

// theme_path: 콘텐츠 폴더 기준 테마 파일 (예: "ui/theme.json")
[[nodiscard]] UiResources load_ui_resources(engine::ResourceManager& resources,
                                            engine::TextRenderer& text, std::string_view theme_path);

}  // namespace wind::client
