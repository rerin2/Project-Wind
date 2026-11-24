#pragma once

// 이 파일은 무엇을 하나요?
// 저장 파일 읽기(save_reader)가 여러 번 쓰는 작은 칸 읽기 도우미들이에요. (16a)
// id 하나, id 목록(겹침 없음), 정해진 수의 칸 목록(id 또는 null), 맵 + x·y 자리.
// 틀리면 오류 공책에 칸 주소와 함께 적고 nullopt(또는 false)예요.

#include "wind/content/json_reader.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::save {

inline constexpr std::size_t kMaxIdLength = 64;  // 아이템·마술·쿠션·깃발 id (바이트)
inline constexpr double kMaxCoordinate = 1e6;    // 맵 위 픽셀 상한

// value가 id 모양(1~64바이트 글자)이면 그 글자. field: 오류에 쓸 칸 주소
[[nodiscard]] std::optional<std::string> id_value(const nlohmann::json& value, content::JsonReader& r,
                                                  const std::string& field);
// key 칸의 id 목록 (최대 max_items개, 겹침 없음)
[[nodiscard]] std::optional<std::vector<std::string>> id_list(content::JsonReader& r, std::string_view key,
                                                              std::size_t max_items);
// key 칸의 정확히 count칸 목록. 칸마다 id 또는 null(빈 칸 → 빈 글자), 든 id는 겹침 없음
[[nodiscard]] std::optional<std::vector<std::string>> slot_ids(content::JsonReader& r, std::string_view key,
                                                               std::size_t count);
// "map"(콘텐츠 폴더 안 위치), "x", "y"(0~10^6) 세 칸. 모두 읽히면 true이고 map·x·y를 채워요
bool map_point(content::JsonReader& r, std::string& map, float& x, float& y);

}  // namespace wind::save
