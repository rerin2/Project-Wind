#pragma once

// 이 파일은 무엇을 하나요?
// 게임 자료(그림, 맵 설명서)는 모두 "콘텐츠 폴더"라는 한 상자 안에 있어요.
// 설명서에 "tilesets/a.json"처럼 파일 위치가 적혀 있을 때, 그 위치가 상자 밖을
// 가리키지 못하게 막는 문지기예요 (명세 5.7).
// 나쁜 설명서가 "../비밀파일"처럼 적어서 컴퓨터의 다른 파일을 몰래 읽는 일을 막아요.

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace wind::content {

// 파일 위치 글자를 깔끔한 모양 "폴더/파일.확장자"로 정리해요.
// '\'는 '/'로 바꾸고, 의미 없는 "./"나 빈칸("a//b")은 지워요.
// 아래 경우는 위험해서 "안 돼요"(nullopt)를 돌려줘요:
//   - 절대 경로(맨 앞이 '/'), 드라이브 글자("C:")
//   - ".."(한 칸 위 폴더로 나가기), 눈에 안 보이는 제어 문자
[[nodiscard]] std::optional<std::string> normalize_content_path(std::string_view relative);

// 정리한 위치를 콘텐츠 폴더(root) 아래의 진짜 파일 위치로 바꿔요. 위험한 위치면 nullopt.
[[nodiscard]] std::optional<std::filesystem::path> resolve_content_path(
    const std::filesystem::path& root, std::string_view relative);

}  // namespace wind::content
