#pragma once

// 이 파일은 무엇을 하나요?
// 화면 왼쪽 아래 알림 줄에 띄울 일 하나예요. (14b, 개발자와 정함)
// "슬라임 젤리 ×2"(주움) 또는 "가방이 가득 찼다"(못 주움). 필드가 모아 두고 앱이 가져가요.

#include "wind/items/item_stack.hpp"

namespace wind::game {

struct LootNotice {
    items::ItemStack stack;  // 무엇이 몇 개
    bool bag_full = false;   // true면 가방이 가득 차서 못 주웠어요
};

}  // namespace wind::game
