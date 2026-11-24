// 이 파일은 무엇을 하나요?
// 저장 파일 칸 읽기 도우미의 실제 동작이에요 (save_fields.hpp 설명 참고).

#include "wind/save/save_fields.hpp"

#include <set>
#include <utility>

namespace wind::save {

using content::JsonReader;
using nlohmann::json;

std::optional<std::string> id_value(const json& value, JsonReader& r, const std::string& field) {
    // 글자가 아니거나, 비었거나, 너무 길면 틀린 id
    if (!value.is_string() || value.get_ref<const std::string&>().empty() ||
        value.get_ref<const std::string&>().size() > kMaxIdLength) {
        r.error(field, "expected an id string of 1-" + std::to_string(kMaxIdLength) + " bytes");
        return std::nullopt;
    }
    return value.get<std::string>();
}

std::optional<std::vector<std::string>> id_list(JsonReader& r, std::string_view key, std::size_t max_items) {
    const auto* list = r.array(key, max_items);
    if (list == nullptr) {
        return std::nullopt;
    }
    std::vector<std::string> ids;
    std::set<std::string, std::less<>> seen;  // 겹침 찾기용
    bool ok = true;
    for (std::size_t i = 0; i < list->size(); ++i) {
        auto id = id_value((*list)[i], r, r.index_path(key, i));
        if (!id) {
            ok = false;
        } else if (!seen.insert(*id).second) {
            r.error(r.index_path(key, i), "duplicate id " + *id);
            ok = false;
        } else {
            ids.push_back(std::move(*id));
        }
    }
    return ok ? std::optional{std::move(ids)} : std::nullopt;
}

std::optional<std::vector<std::string>> slot_ids(JsonReader& r, std::string_view key, std::size_t count) {
    const auto* list = r.array(key, count);
    if (list == nullptr) {
        return std::nullopt;
    }
    // 칸 수가 모자라도 틀린 파일이에요 (칸 번호가 뜻을 가지니까)
    if (list->size() != count) {
        r.error(r.field_path(key), "expected exactly " + std::to_string(count) + " slots");
        return std::nullopt;
    }
    std::vector<std::string> slots(count);
    std::set<std::string, std::less<>> seen;
    bool ok = true;
    for (std::size_t i = 0; i < count; ++i) {
        if ((*list)[i].is_null()) {
            continue;  // 빈 칸은 빈 글자 그대로
        }
        auto id = id_value((*list)[i], r, r.index_path(key, i));
        if (!id) {
            ok = false;
        } else if (!seen.insert(*id).second) {
            r.error(r.index_path(key, i), "id " + *id + " is in two slots");
            ok = false;
        } else {
            slots[i] = std::move(*id);
        }
    }
    return ok ? std::optional{std::move(slots)} : std::nullopt;
}

bool map_point(JsonReader& r, std::string& map, float& x, float& y) {
    // 셋 다 읽어 봐야 오류가 한꺼번에 적혀요 (하나에서 멈추지 않게)
    const auto read_map = r.content_path("map");
    const auto read_x = r.number("x", 0.0, kMaxCoordinate);
    const auto read_y = r.number("y", 0.0, kMaxCoordinate);
    if (!read_map || !read_x || !read_y) {
        return false;
    }
    map = *read_map;
    x = static_cast<float>(*read_x);
    y = static_cast<float>(*read_y);
    return true;
}

}  // namespace wind::save
