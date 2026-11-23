#pragma once

// 이 파일은 무엇을 하나요?
// 인벤토리 화면을 그려요. (명세 6.9, 10.2, 14c)
// 생각(어느 칸을 골랐나, 작은 메뉴)은 game::BagScreen이 하고, 여기는 보여 주기만 해요.
// - 화면 왼쪽 창 (가운데의 주인공과 다가오는 몬스터를 가리지 않게, 게임은 계속 흘러요)
// - 위: "가방"과 쓴 칸 수, 가운데: 6 × 4 칸 (아이콘, 개수, 희귀도 색 테두리, 고른 칸은 금빛),
//   아래: 고른 아이템의 이름(희귀도 색)·종류·효과·설명, 맨 아래: 키 안내
// - 아이템 칸에서 Enter를 누르면 그 칸 오른쪽에 작은 메뉴 (사용 / 버리기 / 취소, 못 고르는 것은 흐리게)
// - 열릴 때 0.18초 동안 왼쪽에서 미끄러지며 나타나요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::items {
class Inventory;
class ItemCatalog;
}  // namespace wind::items
namespace wind::game {
class BagScreen;
}

namespace wind::client {

class BagScreenView {
public:
    // icons: 아이템 아이콘 그림 판 (16×16 칸)
    explicit BagScreenView(engine::TextureHandle icons) : icons_(icons) {}

    // 화면 한 장마다: 열려 있으면 나타나기 시계를 돌려요 (닫히면 0으로)
    void update(float dt_seconds, bool open);
    void draw(engine::UiPainter& painter, engine::Vec2 screen, const game::BagScreen& bag_screen,
              const items::Inventory& bag, const items::ItemCatalog& catalog) const;

private:
    // 칸 하나 (배경, 테두리, 아이콘, 개수)
    void draw_slot(engine::UiPainter& painter, engine::RectF cell, std::size_t slot, bool focused,
                   const items::Inventory& bag, const items::ItemCatalog& catalog, float alpha) const;
    // 고른 아이템 설명 (이름·종류·효과·설명)
    void draw_details(engine::UiPainter& painter, engine::RectF box, std::size_t slot,
                      const items::Inventory& bag, const items::ItemCatalog& catalog, float alpha) const;
    // 작은 메뉴 (사용 / 버리기 / 취소)
    void draw_menu(engine::UiPainter& painter, engine::RectF cell, const game::BagScreen& bag_screen,
                   const items::Inventory& bag, const items::ItemCatalog& catalog) const;

    engine::TextureHandle icons_;
    float appear_ = 0.0f;  // 열릴 때 0 -> 1
};

}  // namespace wind::client
