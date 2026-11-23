#pragma once

// 이 파일은 무엇을 하나요?
// 필드가 빌려 쓰는 "아이템 쪽 물건들" 묶음이에요. (14b)
// 셋 다 필드 화면 밖(ClientApp)에 하나씩 있어서, 맵을 옮기거나 쉬거나 부활해도 그대로예요.
// (HeroProgress처럼 필드는 참조로 빌려 쓰기만 해요)

namespace wind::items {
class Inventory;
class ItemCatalog;
}  // namespace wind::items

namespace wind::game {

class LootStore;

struct ItemWorld {
    items::Inventory& bag;              // 주인공 가방
    const items::ItemCatalog& catalog;  // 아이템 도감
    LootStore& loot;                    // 맵마다의 바닥 아이템
};

}  // namespace wind::game
