#pragma once

// 이 파일은 무엇을 하나요?
// 파일이나 글자를 JSON(제이슨: 정해진 양식으로 쓴 설명서)으로 바꾸는 도우미 함수들이에요.
// 실패해도 프로그램이 멈추지 않고, 이유를 오류 공책에 적어요.

#include "wind/content/error_list.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace wind::content {

// 설명서 파일의 최대 크기 (8MB).
// 엄청나게 큰 파일을 읽느라 컴퓨터가 멈추지 않게, 읽기 전에 크기부터 확인해요.
inline constexpr std::uintmax_t kMaxContentFileBytes = 8u * 1024u * 1024u;

// 파일을 읽어 JSON으로 바꿔요. 실패하면 이유를 공책에 적어요.
// display_name: 오류 메시지에 보여 줄 파일 이름 (보통 콘텐츠 폴더 기준 위치)
[[nodiscard]] std::optional<nlohmann::json> read_json_file(const std::filesystem::path& path,
                                                           const std::string& display_name,
                                                           ErrorList& errors);

// 글자를 바로 JSON으로 바꿔요 (테스트나 도구에서 써요).
[[nodiscard]] std::optional<nlohmann::json> parse_json_text(std::string_view text,
                                                            const std::string& display_name,
                                                            ErrorList& errors);

}  // namespace wind::content
