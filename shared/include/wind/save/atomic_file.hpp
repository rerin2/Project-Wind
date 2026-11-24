#pragma once

// 이 파일은 무엇을 하나요?
// 파일을 "한 번에" 바꾸는 도우미예요. (명세 12.2 원자적 쓰기, docs/SAVE_FORMAT.md 2절, 16a)
// 새 내용을 옆의 임시 파일(<이름>.tmp)에 다 쓰고 닫은 뒤, 이름을 바꿔 본 파일을 덮어써요.
// 쓰는 도중 게임이 꺼져도 본 파일은 이전 내용 그대로예요(반쯤 쓴 것은 .tmp에만 남아요).
// 이름 바꾸기는 같은 폴더 안이라 Windows에서 MoveFileEx(교체)로 한 번에 일어나요.

#include <filesystem>
#include <string>
#include <string_view>

namespace wind::save {

// target을 bytes로 바꿔요. 성공하면 true. 실패하면 target은 그대로이고 error에 이유를 적어요.
bool write_file_atomically(const std::filesystem::path& target, std::string_view bytes, std::string& error);

// from 파일을 target으로 복사해요 (같은 방식: target.tmp로 복사한 뒤 이름 바꾸기). 백업 만들기·되살리기에 써요.
bool copy_file_atomically(const std::filesystem::path& from, const std::filesystem::path& target, std::string& error);

// target 옆 임시 파일 위치 (<이름>.tmp)
[[nodiscard]] std::filesystem::path temp_path_for(const std::filesystem::path& target);

}  // namespace wind::save
