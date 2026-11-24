#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴를 게임 루프에 잇는 곳이에요. (명세 13.1, 13.4, 15e)
// - 닫혀 있을 때: Esc·Tab이면 마지막으로 보던 탭으로, I면 가방 탭으로 열어요 (열 수 있을 때만).
// - 열려 있을 때: 방향키·Enter·E·J(확인)·R(정렬)·Esc(뒤로)·Tab(다음 탭)을 game::PauseMenu에 넘기고,
//   I는 가방 탭이면 닫고 다른 탭이면 가방 탭으로 옮겨요.
// - 가방의 "사용"·"장착"·"버리기"·"정렬"은 필드에 요청하고, 필드가 다음 박자에 규칙대로 처리해요.
//   "사용"·"장착"을 고르면 메뉴가 닫혀서 마시는 모습·알림이 보여요 (14c부터 같은 규칙).
// - 쓰러지거나, 쉬거나, 맵을 옮기거나, 쿠션 메뉴가 열리면 저절로 닫혀요 (can_open = false).
// - 그리기는 PauseMenuView가 해요.

#include "app/pause_menu_view.hpp"
#include "engine/input/input_state.hpp"
#include "game/input/input_bindings.hpp"
#include "game/ui/pause_menu.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class FieldState;
class HeroProgress;
}  // namespace wind::game

namespace wind::client {

struct ItemSetup;

class PauseMenuPanel {
public:
    // items: 가방·도감·장비·무기·마술 선반, progress: 능력치·마술 공책
    // icons: 아이템 아이콘 그림 판, spell_icons: 마술 아이콘 그림 판
    PauseMenuPanel(ItemSetup& items, const game::HeroProgress& progress, engine::TextureHandle icons,
                   engine::TextureHandle spell_icons);

    // 키 확인 (필드 입력보다 먼저). can_open: 지금 메뉴를 열 수 있나.
    // "게임 끄기"를 골랐으면 true
    bool handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                      game::FieldState& field, bool can_open);
    [[nodiscard]] bool is_open() const { return menu_.is_open(); }
    // 메뉴가 게임 입력을 막고 있나요? (명세 13.3)
    [[nodiscard]] bool captures_input() const { return menu_.captures_input(); }
    // 화면 한 장마다: 열 수 없게 됐으면 닫고, 나타나기 시계를 돌려요
    void update(float dt_seconds, bool can_open);
    void draw(engine::UiPainter& painter, engine::Vec2 screen) const;

private:
    // 열려 있을 때 키 하나씩 메뉴에 넘기고, 가방 명령을 필드에 요청해요
    bool handle_open(const engine::InputState& input, const game::InputBindings& bindings, game::FieldState& field);

    ItemSetup& items_;
    const game::HeroProgress& progress_;
    game::PauseMenu menu_;
    PauseMenuView view_;
};

}  // namespace wind::client
