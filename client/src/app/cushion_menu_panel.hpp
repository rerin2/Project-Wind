#pragma once

// 이 파일은 무엇을 하나요?
// 쿠션 메뉴를 게임 루프에 잇는 곳이에요. (13c)
// - 키(방향키·Enter·E·Esc)를 메뉴 입력으로 바꿔 game::CushionMenu에 넘겨요.
// - 결과를 필드에 적용해요: "떠나기"면 주인공이 기지개 켜고 일어나고,
//   레벨업이면 새 능력치를 바로 적용하고(바가 늘어나요) 빛기둥과 "레벨 N" 배너를 띄워요.
//   마술 칸이 바뀌면 주인공 손의 마술을 다시 쥐여 줘요 (15f).
// - 그리기는 CushionMenuView가 해요. 열려 있는 동안 게임 루프는 박자를 돌리지 않아요(시간 정지).

#include "app/cushion_menu_view.hpp"
#include "engine/input/input_state.hpp"
#include "game/input/input_bindings.hpp"
#include "game/ui/cushion_menu.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class FieldState;
class HeroProgress;
class SpellShelf;
}  // namespace wind::game

namespace wind::client {

class FieldUi;

class CushionMenuPanel {
public:
    void open() { menu_.open(); }
    // 마술 선반과 아이콘 판 (15f): "마술 칸" 화면이 요구 지능·이름·아이콘을 봐요. 선반은 이 패널보다 오래 살아 있어야 해요.
    void set_spells(const game::SpellShelf& shelf, engine::TextureHandle icons) {
        menu_.set_spell_shelf(shelf);
        view_.set_spells(&shelf, icons);
    }
    [[nodiscard]] bool is_open() const { return menu_.is_open(); }
    // 열려 있으면 키를 처리해요. Esc를 메뉴가 썼으면 true (게임을 끄지 않아요)
    bool handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                      game::HeroProgress& progress, game::FieldState& field, FieldUi& ui);
    void update(float dt_seconds) { view_.update(dt_seconds, menu_); }
    void draw(engine::UiPainter& painter, engine::Vec2 screen, const game::HeroProgress& progress) const {
        view_.draw(painter, screen, menu_, progress);
    }

private:
    game::CushionMenu menu_;
    CushionMenuView view_;
};

}  // namespace wind::client
