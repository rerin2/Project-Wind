// 이 파일은 무엇을 하나요?
// 아이템 알림 줄: 줄 만들기·합치기·나이 먹기·사라지기.

#include "game/ui/loot_toasts.hpp"

#include "wind/items/item_catalog.hpp"

#include <algorithm>

namespace wind::game {
namespace {

constexpr const char* kTimes = " ×";                // 개수 앞 표시
constexpr const char* kBagFull = "가방이 가득 찼다 · "; // 가득 참 줄 앞부분

// 줄 글 만들기: "이름 ×개수" 또는 "가방이 가득 찼다 · 이름"
std::string make_text(const std::string& name, std::int32_t count, bool bag_full) {
    if (bag_full) return kBagFull + name;
    return name + kTimes + std::to_string(count);
}

}  // namespace

void LootToasts::push(const LootNotice& notice, const items::ItemCatalog& catalog) {
    const auto* def = catalog.find(notice.stack.item_id);
    const std::string name = def != nullptr ? def->name : notice.stack.item_id;  // 모르면 id라도 보여 줘요
    // 방금 같은 아이템을 주웠으면 개수만 더하고 다시 처음부터 보여 줘요
    if (!notice.bag_full && !lines_.empty()) {
        auto& last = lines_.back();
        if (!last.bag_full && last.item_id == notice.stack.item_id && last.age < kMergeSeconds) {
            last.count += notice.stack.count;
            last.text = make_text(name, last.count, false);
            last.age = kSlideSeconds;  // 이미 들어와 있으니 미끄러지지 않고 시간만 다시
            return;
        }
    }
    lines_.push_back({notice.stack.item_id, notice.stack.count, make_text(name, notice.stack.count, notice.bag_full),
                      def != nullptr ? def->icon : 0, notice.bag_full, 0.0f});
    if (lines_.size() > kMaxLines) lines_.erase(lines_.begin());  // 너무 많으면 가장 오래된 줄부터
}

void LootToasts::update(float dt_seconds) {
    for (auto& line : lines_) line.age += dt_seconds;
    std::erase_if(lines_, [](const Line& line) { return line.age >= kLifeSeconds; });
}

float LootToasts::slide(const Line& line) {
    const float u = std::clamp(line.age / kSlideSeconds, 0.0f, 1.0f);
    return 1.0f - (1.0f - u) * (1.0f - u);  // 빨리 들어오다 끝에서 부드럽게 멈춰요
}

float LootToasts::alpha(const Line& line) {
    return std::clamp((kLifeSeconds - line.age) / kFadeSeconds, 0.0f, 1.0f);  // 끝 0.4초 동안 흐려져요
}

}  // namespace wind::game
