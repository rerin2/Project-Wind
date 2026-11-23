#pragma once

// 이 파일은 무엇을 하나요?
// 필드의 무기 담당이에요. (15a) 주인공 손에 "지금 끼고 있는 무기"를 쥐여 줘요.
// - arm: 장비 칸의 무기 id → 무기 선반에서 무기 → 힘으로 무거운지 보고(derive_stats) → 무거우면 느린 사본(wield)
//   필드를 만들 때, 레벨업 뒤(힘이 바뀌면 무거움도 바뀌어요), 장착한 뒤에 불러요.
// - 인벤토리 화면의 "장착" 요청: 다음 박자에 규칙대로 처리해요.
//   주인공이 서기·걷기·달리기 중이 아니면 "지금은 바꿀 수 없다", 되면 바꿔 끼우고 "단검 장착"
//   (무거우면 붉은 "대검 장착 · 너무 무겁다"). 실패하면 가방·장비 모두 그대로예요 (전부 성공 또는 전부 실패).
// 장비·가방·도감·무기 선반·진행 상태는 필드 밖(ClientApp)에 있고 여기서는 빌려 써요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "game/items/gear_world.hpp"
#include "game/items/item_world.hpp"
#include "game/items/loot_notice.hpp"

#include "wind/combat/weapon.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace wind::game {

class HeroProgress;
class PlayerMotion;

// 손에 든 무기 (무거움 반영)
struct HeldWeapon {
    combat::Weapon weapon;  // 무거우면 시간·스태미나가 늘어난 사본, 맨손이면 빈 무기
    bool heavy = false;     // 힘이 모자라 무거운가 (명세 8.4)
};

class FieldArms {
public:
    FieldArms(const ItemWorld& items, const GearWorld& gear, const HeroProgress& progress)
        : items_(items), gear_(gear), progress_(progress) {}

    // 지금 끼고 있는 무기 (장비 칸이 비었거나 모르는 무기면 맨손)
    [[nodiscard]] HeldWeapon held() const;
    // 주인공 손에 지금 무기를 쥐여 줘요
    void arm(PlayerMotion& hero) const;

    // 인벤토리 화면 "장착" (다음 박자에 처리해요)
    void request_equip(std::size_t bag_slot) { requests_.push_back(bag_slot); }
    // 정해진 박자마다: 장착 요청 처리
    void step(PlayerMotion& hero);
    // 모아 둔 알림을 가져가요 (가져가면 비워요)
    [[nodiscard]] std::vector<LootNotice> take_notices() { return std::exchange(notices_, {}); }

private:
    void equip(std::size_t bag_slot, PlayerMotion& hero);

    ItemWorld items_;                  // 가방·도감 (빌려 씀)
    GearWorld gear_;                   // 장비·무기 선반 (빌려 씀)
    const HeroProgress& progress_;     // 힘 → 들 수 있는 무게
    std::vector<std::size_t> requests_;  // 다음 박자에 낄 가방 칸들
    std::vector<LootNotice> notices_;    // 알림 줄에 띄울 일
};

}  // namespace wind::game
