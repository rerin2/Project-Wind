// 이 파일은 무엇을 하나요?
// json_reader.hpp의 "설명서 숙제 검사 도우미"가 실제로 칸을 하나씩 확인하는 곳이에요.
// 규칙은 늘 같아요: 칸을 찾고 -> 모양(글자/숫자/목록)이 맞는지 보고 -> 크기가 괜찮은지 본다.
// 하나라도 틀리면 공책(ErrorList)에 적고 "없음"을 돌려줘요.

#include "wind/content/json_reader.hpp"

#include <cmath>
#include <string>

namespace wind::content {

// 도우미를 만들 때 받은 것이 { ... } 묶음인지 먼저 확인해요.
JsonReader::JsonReader(const nlohmann::json& object, std::string file, std::string path,
                       ErrorList& errors)
    : object_(object), file_(std::move(file)), path_(std::move(path)), errors_(errors) {
    if (!object_.is_object()) {
        valid_ = false;
        error(path_, "expected an object");
    }
}

// 맨 바깥 묶음이면 칸 이름 그대로, 안쪽 묶음이면 "바깥주소.칸이름"
std::string JsonReader::field_path(std::string_view key) const {
    return path_.empty() ? std::string(key) : path_ + "." + std::string(key);
}

std::string JsonReader::index_path(std::string_view key, std::size_t index) const {
    return field_path(key) + "[" + std::to_string(index) + "]";
}

void JsonReader::error(std::string field, std::string message) {
    errors_.add(file_, std::move(field), std::move(message));
}

bool JsonReader::has(std::string_view key) const {
    return valid_ && object_.contains(key);
}

// 꼭 있어야 하는 칸을 찾아요. 없으면 공책에 적어요.
const nlohmann::json* JsonReader::find(std::string_view key) {
    if (!valid_) {
        return nullptr;
    }
    const auto it = object_.find(key);
    if (it == object_.end()) {
        error(field_path(key), "required field is missing");
        return nullptr;
    }
    return &*it;
}

std::optional<std::string> JsonReader::string(std::string_view key, std::size_t max_length) {
    const auto* value = find(key);
    if (value == nullptr) {
        return std::nullopt;
    }
    if (!value->is_string()) {
        error(field_path(key), "expected a string");
        return std::nullopt;
    }
    auto text = value->get<std::string>();
    // 빈 글자나 너무 긴 글자는 실수일 가능성이 커서 받지 않아요.
    if (text.empty() || text.size() > max_length) {
        error(field_path(key), "string length must be 1-" + std::to_string(max_length));
        return std::nullopt;
    }
    return text;
}

std::optional<std::int64_t> JsonReader::integer(std::string_view key, std::int64_t min,
                                                std::int64_t max) {
    const auto* value = find(key);
    if (value == nullptr) {
        return std::nullopt;
    }
    // 소수(1.5)나 따옴표 속 숫자("3")는 정수로 인정하지 않아요.
    if (!value->is_number_integer()) {
        error(field_path(key), "expected an integer");
        return std::nullopt;
    }
    const auto number = value->get<std::int64_t>();
    // 허락된 범위 밖이면 틀린 것 (예: 맵 가로 칸 수가 0이거나 너무 큼)
    if (number < min || number > max) {
        error(field_path(key),
              "value " + std::to_string(number) + " is outside " + std::to_string(min) + "-" +
                  std::to_string(max));
        return std::nullopt;
    }
    return number;
}

std::optional<double> JsonReader::number(std::string_view key, double min, double max) {
    const auto* value = find(key);
    if (value == nullptr) {
        return std::nullopt;
    }
    // 따옴표 속 숫자("3")나 참/거짓은 숫자로 인정하지 않아요.
    if (!value->is_number()) {
        error(field_path(key), "expected a number");
        return std::nullopt;
    }
    const auto number = value->get<double>();
    // NaN(숫자가 아님)이나 무한대는 범위 비교가 이상하게 동작해서 따로 막아요.
    if (!std::isfinite(number) || number < min || number > max) {
        error(field_path(key), "value " + std::to_string(number) + " is outside " +
                                   std::to_string(min) + "-" + std::to_string(max));
        return std::nullopt;
    }
    return number;
}

std::optional<bool> JsonReader::boolean(std::string_view key) {
    const auto* value = find(key);
    if (value == nullptr) {
        return std::nullopt;
    }
    if (!value->is_boolean()) {
        error(field_path(key), "expected true or false");
        return std::nullopt;
    }
    return value->get<bool>();
}

const nlohmann::json* JsonReader::array(std::string_view key, std::size_t max_items) {
    const auto* value = find(key);
    if (value == nullptr) {
        return nullptr;
    }
    if (!value->is_array()) {
        error(field_path(key), "expected an array");
        return nullptr;
    }
    // 항목이 너무 많은 목록은 컴퓨터를 힘들게 하니 거절해요.
    if (value->size() > max_items) {
        error(field_path(key), "array has " + std::to_string(value->size()) +
                                   " items, maximum is " + std::to_string(max_items));
        return nullptr;
    }
    return value;
}

const nlohmann::json* JsonReader::object(std::string_view key) {
    const auto* value = find(key);
    if (value == nullptr) {
        return nullptr;
    }
    if (!value->is_object()) {
        error(field_path(key), "expected an object");
        return nullptr;
    }
    return value;
}

}  // namespace wind::content
