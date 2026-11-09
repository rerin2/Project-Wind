#pragma once

// 이 파일은 무엇을 하나요?
// 자료 파일에 적힌 색 글자("#ffb060" 또는 "#ffb06080")를 색으로 바꾸는 도우미예요.
// #rrggbb: 빨강·초록·파랑을 두 글자씩 16진수(0~9, a~f)로 적어요. 뒤에 aa를 붙이면 진하기예요.

#include "wind/world/rgba.hpp"

#include <optional>
#include <string_view>

namespace wind::content {

// 모양이 틀리면 nullopt. aa가 없으면 진하기는 255(완전히 진함)예요.
[[nodiscard]] std::optional<world::Rgba> parse_hex_color(std::string_view text);

}  // namespace wind::content
