#pragma once

// 이 파일은 무엇을 하나요?
// 필드 위에 얹는 아이템 HUD를 한데 모아 게임 루프에 이어요. (14b 알림 줄, 15b 무기·마술 칸)
// - 왼쪽 아래 무기·마술 칸(HandsHud, 15b)과 그 위 알림 줄(LootToasts): 필드가 모아 둔 줍기·가득 참 알림을 가져와 보여 줘요.
// - 아래 가운데 단축 칸 1~4(QuickSlotsHud, 15f): 등록된 아이템의 아이콘과 가방 속 개수.
// - 인벤토리 화면은 15e부터 일시정지 메뉴의 "가방" 탭이에요 (PauseMenuPanel).

#include "app/hands_hud.hpp"
#include "app/quick_slots_hud.hpp"
#include "game/ui/loot_toasts.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class FieldState;
class HeroProgress;
}

namespace wind::client {

struct ItemSetup;

class ItemHud {
public:
    // items: 도감·장비 (앱이 가지고 있어요), progress: 마술 공책·지능 (15b)
    // icons: 아이템 아이콘 그림 판, spell_icons: 마술 아이콘 그림 판 (15b)
    ItemHud(ItemSetup& items, const game::HeroProgress& progress, engine::TextureHandle icons,
            engine::TextureHandle spell_icons);

    // 화면 한 장마다: 필드의 알림을 가져오고 시계를 돌려요
    void update(float dt_seconds, game::FieldState& field);
    // 무기·마술 칸, 그 위 알림 줄
    void draw(engine::UiPainter& painter, engine::Vec2 screen) const;

private:
    // 무기·마술 칸이 보여 줄 것을 모아요 (15b)
    [[nodiscard]] HandsInfo hands_info(const game::FieldState& field) const;
    // 단축 칸이 보여 줄 것을 모아요: 등록된 아이템의 아이콘과 가방 속 개수 (15f)
    [[nodiscard]] std::array<QuickSlotInfo, QuickSlotsHud::kCount> quick_info() const;

    ItemSetup& items_;
    const game::HeroProgress& progress_;  // 마술 공책·지능 (15b)
    engine::TextureHandle icons_;
    HandsHud hands_;            // 왼쪽 아래 무기·마술 칸 (15b)
    QuickSlotsHud quick_;       // 아래 가운데 단축 칸 1~4 (15f)
    game::LootToasts toasts_;   // 왼쪽 아래 알림 줄 (14b)
};

}  // namespace wind::client
