#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴를 그려요. (명세 6.9, 13.1, 15e) 생각은 game::PauseMenu가 하고 여기는 보여 주기만 해요.
// - 화면 왼쪽 창 (주인공과 다가오는 몬스터를 가리지 않게, 게임은 계속 흘러요. 개발자와 정함)
// - 맨 위: 탭 다섯 개 (가방 · 장비 · 마술 · 능력치 · 시스템). 고른 탭은 금빛 굵은 글자와 밑줄
// - 가운데: 고른 탭의 페이지 (가방은 BagScreenView, 장비·마술·능력치는 menu_*_page, 시스템은 여기)
// - 맨 아래: 키 안내
// - 열릴 때 0.18초 동안 왼쪽에서 미끄러지며 나타나고, 탭을 바꾸면 페이지가 0.12초 동안 살짝 올라오며 나타나요.

#include "app/bag_screen_view.hpp"
#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"
#include "game/ui/pause_menu.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class HeroProgress;
}

namespace wind::client {

struct ItemSetup;
struct MenuPage;

class PauseMenuView {
public:
    // icons: 아이템 아이콘 판, spell_icons: 마술 아이콘 판
    PauseMenuView(engine::TextureHandle icons, engine::TextureHandle spell_icons);

    // 화면 한 장마다: 나타나기·탭 바꾸기 시계를 돌려요 (닫히면 0으로)
    void update(float dt_seconds, const game::PauseMenu& menu);
    void draw(engine::UiPainter& painter, engine::Vec2 screen, const game::PauseMenu& menu, const ItemSetup& items,
              const game::HeroProgress& progress) const;

private:
    // 탭 줄 (고른 탭 금빛 + 밑줄)
    void draw_tabs(engine::UiPainter& painter, engine::RectF row, game::MenuTab tab, float alpha) const;
    // 시스템 탭: 설정 · 타이틀로 (흐림) · 게임 끄기, 저장 안내
    static void draw_system(const MenuPage& page, game::SystemItem focused);

    BagScreenView bag_view_;
    engine::TextureHandle icons_;
    engine::TextureHandle spell_icons_;
    float appear_ = 0.0f;     // 열릴 때 0 -> 1
    float tab_fade_ = 1.0f;   // 탭을 바꾼 뒤 0 -> 1
    game::MenuTab shown_tab_ = game::MenuTab::Bag;  // 지난 장에 보인 탭 (바뀌었는지 보려고)
};

}  // namespace wind::client
