#pragma once

// 이 파일은 무엇을 하나요?
// 필드가 빌려 쓰는 "장비 쪽 물건들" 묶음이에요. (15a)
// 둘 다 필드 화면 밖(ClientApp)에 하나씩 있어서, 맵을 옮기거나 쉬거나 부활해도 그대로예요 (ItemWorld와 같아요).

namespace wind::items {
class Equipment;
}

namespace wind::game {

class WeaponRack;

struct GearWorld {
    items::Equipment& worn;      // 주인공이 끼고 있는 장비
    const WeaponRack& weapons;   // 무기 아이템 → 미리 읽은 무기
};

}  // namespace wind::game
