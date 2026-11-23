// 이 파일은 무엇을 하나요?
// 아이템 알림 줄: 줄 만들기·합치기·나이 먹기·사라지기.

#include "game/ui/loot_toasts.hpp"

#include "wind/items/item_catalog.hpp"

#include <algorithm>

namespace wind::game {
namespace {

constexpr const char* kTimes = " ×";                      // 개수 앞 표시
constexpr const char* kBagFull = "가방이 가득 찼다 · ";     // 가득 참 줄 앞부분
constexpr const char* kCannotUse = "지금은 마실 수 없다";   // 공격·구르기·맞는 중 (14d)
constexpr const char* kAlreadyFull = "이미 가득하다 · ";    // 채울 HP·MP가 없음 (14d)
constexpr const char* kEquipped = " 장착";                 // 무기를 바꿔 낌 (15a)
constexpr const char* kHeavy = " 장착 · 너무 무겁다";      // 힘이 모자라 무거움 (15a)
constexpr const char* kCannotEquip = "지금은 바꿀 수 없다";  // 공격·구르기·맞는 중 (15a)

// 줄 글 만들기: 알림 종류마다 한 줄
std::string make_text(const std::string& name, std::int32_t count, LootNotice::Kind kind) {
    switch (kind) {
        case LootNotice::Kind::BagFull: return kBagFull + name;
        case LootNotice::Kind::CannotUse: return kCannotUse;
        case LootNotice::Kind::AlreadyFull: return kAlreadyFull + name;
        case LootNotice::Kind::Equipped: return name + kEquipped;
        case LootNotice::Kind::EquippedHeavy: return name + kHeavy;
        case LootNotice::Kind::CannotEquip: return kCannotEquip;
        case LootNotice::Kind::PickedUp: break;
    }
    return name + kTimes + std::to_string(count);
}

}  // namespace

void LootToasts::push(const LootNotice& notice, const items::ItemCatalog& catalog) {
    const auto* def = catalog.find(notice.stack.item_id);
    const std::string name = def != nullptr ? def->name : notice.stack.item_id;  // 모르면 id라도 보여 줘요
    // 방금 같은 줄이 있으면 새 줄 대신: 주운 것은 개수를 더하고, 경고는 시간만 다시 (같은 경고가 쌓이지 않게)
    if (!lines_.empty()) {
        auto& last = lines_.back();
        if (last.kind == notice.kind && last.item_id == notice.stack.item_id && last.age < kMergeSeconds) {
            if (!notice.warning()) last.count += notice.stack.count;
            last.text = make_text(name, last.count, notice.kind);
            last.age = kSlideSeconds;  // 이미 들어와 있으니 미끄러지지 않고 시간만 다시
            return;
        }
    }
    lines_.push_back({notice.stack.item_id, notice.stack.count, make_text(name, notice.stack.count, notice.kind),
                      def != nullptr ? def->icon : 0, notice.kind, 0.0f});
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
