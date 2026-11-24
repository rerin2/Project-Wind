#pragma once

// 이 파일은 무엇을 하나요?
// 인벤토리 화면을 그려요. (명세 6.9, 10.2, 14c)
// 생각(어느 칸을 골랐나, 작은 메뉴)은 game::BagScreen이 하고, 여기는 보여 주기만 해요.
// - 15e부터 일시정지 메뉴 창(PauseMenuView)의 "가방" 탭 안에 그려요. 창·나타나기·키 안내는 메뉴가 맡아요.
// - 위: "가방"과 쓴 칸 수, 가운데: 6 × 4 칸 (아이콘, 개수, 희귀도 색 테두리, 고른 칸은 금빛),
//   아래: 고른 아이템의 이름(희귀도 색)·종류·효과·설명
// - 아이템 칸에서 Enter를 누르면 그 칸 오른쪽에 작은 메뉴 (사용 / 버리기 / 취소, 못 고르는 것은 흐리게)

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::items {
class Inventory;
class ItemCatalog;
class QuickSlots;
}  // namespace wind::items
namespace wind::game {
class HeroProgress;
class WeaponRack;
class BagScreen;
}

namespace wind::client {

class BagScreenView {
public:
    // icons: 아이템 아이콘 그림 판 (16×16 칸)
    explicit BagScreenView(engine::TextureHandle icons) : icons_(icons) {}

    // 그릴 자리 크기 (가로 = 칸 6개 폭, 세로 = 칸 수 줄 + 칸 + 설명)
    [[nodiscard]] static engine::Vec2 size(const engine::UiPainter& painter, const items::Inventory& bag);
    // origin(왼쪽 위)부터 그려요. alpha: 나타나는 정도, show_menu: 작은 메뉴를 그려도 되나 (다 나타난 뒤)
    void draw(engine::UiPainter& painter, engine::Vec2 origin, float alpha, bool show_menu,
              const game::BagScreen& bag_screen, const items::Inventory& bag, const items::ItemCatalog& catalog) const;
    // 무기 설명 줄에 쓸 무기 선반과 진행 상태 (15a: "무게 14 / 8 · 무거움", 무거운지는 한 곳의 계산 함수로)
    void set_weapon_info(const game::WeaponRack* weapons, const game::HeroProgress* progress) {
        weapons_ = weapons;
        progress_ = progress;
    }
    // 단축 칸 (15f): 등록된 아이템 칸 왼쪽 위에 작은 번호(1~4)를 붙여요 (없으면 안 붙여요)
    void set_quick_slots(const items::QuickSlots* quick) { quick_ = quick; }

private:
    // 칸 하나 (배경, 테두리, 아이콘, 개수)
    void draw_slot(engine::UiPainter& painter, engine::RectF cell, std::size_t slot, bool focused,
                   const items::Inventory& bag, const items::ItemCatalog& catalog, float alpha) const;
    // 고른 아이템 설명 (이름·종류·효과·설명)
    void draw_details(engine::UiPainter& painter, engine::RectF box, std::size_t slot,
                      const items::Inventory& bag, const items::ItemCatalog& catalog, float alpha) const;
    // 작은 메뉴 (사용 또는 장착 / 버리기 / 취소)
    void draw_menu(engine::UiPainter& painter, engine::RectF cell, const game::BagScreen& bag_screen,
                   const items::Inventory& bag, const items::ItemCatalog& catalog) const;

    engine::TextureHandle icons_;
    const game::WeaponRack* weapons_ = nullptr;  // 무기 무게를 찾는 곳 (없으면 무게를 안 보여 줘요)
    const game::HeroProgress* progress_ = nullptr;  // 힘 → 들 수 있는 무게, 무거움
    const items::QuickSlots* quick_ = nullptr;      // 단축 칸 번호 붙이기 (15f)
};

}  // namespace wind::client
