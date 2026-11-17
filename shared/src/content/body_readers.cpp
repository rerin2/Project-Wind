// 이 파일은 무엇을 하나요?
// 맞는 상자와 그림 띠 칸을 한 칸씩 확인하며 읽는 곳이에요. (training_dummy_loader.cpp에서 옮겨 왔어요)

#include "wind/content/body_readers.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace wind::content {
namespace {

constexpr double kMaxOffset = 128.0;     // 허트박스가 발에서 떨어질 수 있는 거리 (픽셀)
constexpr double kMaxBoxSize = 128.0;    // 허트박스 최대 크기 (픽셀)
constexpr std::int64_t kMaxFrame = 128;  // 그림 한 칸 최대 크기 (픽셀)

// [a, b] 두 숫자 목록 읽기. 둘 다 min~max 안의 숫자여야 해요. 틀린 숫자는 "key[i]" 주소로 적어요.
std::optional<std::array<double, 2>> read_pair(JsonReader& reader, std::string_view key, double min,
                                               double max) {
    const auto* list = reader.array(key, 2);  // 칸이 없거나 목록이 아니면 오류를 적어 줘요
    if (list == nullptr) return std::nullopt;
    if (list->size() != 2) {                  // 꼭 두 개
        reader.error(reader.field_path(key), "expected [a, b]");
        return std::nullopt;
    }
    std::array<double, 2> out{};
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& value = (*list)[i];                                  // i번째 숫자
        if (!value.is_number() || value.get<double>() < min || value.get<double>() > max) {
            reader.error(reader.index_path(key, i),
                         "must be a number " + std::to_string(min) + " to " + std::to_string(max));
            return std::nullopt;
        }
        out[i] = value.get<double>();
    }
    return out;
}

}  // namespace

std::optional<combat::HurtboxShape> read_hurtbox(JsonReader& reader) {
    const auto* object = reader.object("hurtbox");
    if (object == nullptr) return std::nullopt;
    JsonReader box(*object, reader.file(), reader.field_path("hurtbox"), reader.errors());
    const auto offset = read_pair(box, "offset", -kMaxOffset, kMaxOffset);  // 발에서 왼쪽 위 모서리까지
    const auto size = read_pair(box, "size", 1.0, kMaxBoxSize);             // 가로·세로
    if (!offset || !size) return std::nullopt;
    return combat::HurtboxShape{static_cast<float>((*offset)[0]), static_cast<float>((*offset)[1]),
                                static_cast<float>((*size)[0]), static_cast<float>((*size)[1])};
}

std::optional<combat::SpriteStrip> read_sprite_strip(JsonReader& reader) {
    const auto* object = reader.object("sprite");
    if (object == nullptr) return std::nullopt;
    JsonReader sprite(*object, reader.file(), reader.field_path("sprite"), reader.errors());
    auto image = sprite.content_path("image");                                           // 그림 파일 (폴더 안)
    const auto frame = read_pair(sprite, "frame", 1.0, static_cast<double>(kMaxFrame));  // 한 칸 크기
    const auto origin = read_pair(sprite, "origin", 0.0, static_cast<double>(kMaxFrame));  // 발 위치
    if (!image || !frame || !origin) return std::nullopt;
    if ((*origin)[0] > (*frame)[0] || (*origin)[1] > (*frame)[1]) {  // 발 위치는 칸 안이어야 해요
        sprite.error(sprite.field_path("origin"), "origin must be inside the frame");
        return std::nullopt;
    }
    return combat::SpriteStrip{std::move(*image), static_cast<int>((*frame)[0]),
                               static_cast<int>((*frame)[1]), static_cast<int>((*origin)[0]),
                               static_cast<int>((*origin)[1])};
}

}  // namespace wind::content
