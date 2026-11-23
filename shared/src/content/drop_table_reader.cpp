// 이 파일은 무엇을 하나요?
// 드롭 표("drops") 줄마다 아이템·확률·개수를 검사하며 읽어요.

#include "wind/content/drop_table_reader.hpp"

#include <cmath>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxDrops = 8;        // 드롭 표 줄 상한
constexpr std::size_t kMaxIdLength = 64;    // 아이템 id 길이 상한 (도감과 같아요)
constexpr double kMaxDropCount = 99.0;      // 한 번에 나오는 개수 상한

// 줄 하나 읽기: { "item": "...", "chance": 0.5, "count": [1, 2] }. 틀리면 공책에 적고 nullopt.
std::optional<items::DropEntry> read_entry(JsonReader& line) {
    if (!line.valid()) return std::nullopt;  // { ... }가 아니면 만들 때 공책에 적혔어요
    auto item = line.string("item", kMaxIdLength);
    const auto chance = line.number("chance", 0.0, 1.0);
    // 개수는 선택 칸이에요 (없으면 1개)
    std::optional<NumberRange> count = NumberRange{1.0, 1.0};
    if (line.has("count")) count = line.range("count", 1.0, kMaxDropCount);
    if (!item || !chance || !count) return std::nullopt;
    if (*chance <= 0.0) {  // 0%면 절대 안 나오는 줄이라 실수일 거예요
        line.error(line.field_path("chance"), "chance must be above 0");
        return std::nullopt;
    }
    if (std::floor(count->min) != count->min || std::floor(count->max) != count->max) {
        line.error(line.field_path("count"), "count must be whole numbers");
        return std::nullopt;
    }
    return items::DropEntry{std::move(*item), static_cast<float>(*chance), static_cast<std::int32_t>(count->min),
                            static_cast<std::int32_t>(count->max)};
}

}  // namespace

std::optional<std::vector<items::DropEntry>> read_drop_table(JsonReader& reader) {
    std::vector<items::DropEntry> drops;
    if (!reader.has("drops")) return drops;  // 선택 칸: 없으면 아무것도 안 떨어뜨려요
    const auto* list = reader.array("drops", kMaxDrops);  // 목록이 아니거나 너무 길면 공책에 적혀요
    if (list == nullptr) return std::nullopt;
    bool ok = true;
    for (std::size_t i = 0; i < list->size(); ++i) {
        JsonReader line(list->at(i), reader.file(), reader.index_path("drops", i), reader.errors());
        auto entry = read_entry(line);
        if (!entry) {
            ok = false;  // 다른 줄도 마저 검사해서 틀린 곳을 한 번에 다 알려 줘요
            continue;
        }
        drops.push_back(std::move(*entry));
    }
    if (!ok) return std::nullopt;
    return drops;
}

}  // namespace wind::content
