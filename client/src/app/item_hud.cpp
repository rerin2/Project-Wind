// 이 파일은 무엇을 하나요?
// 아이템 UI: 키 -> 인벤토리 화면 입력, 화면 명령 -> 필드 요청, 알림 가져오기, 그리기.

#include "app/item_hud.hpp"

#include "app/item_setup.hpp"
#include "app/loot_toast_drawing.hpp"
#include "game/states/field_state.hpp"

#include <array>
#include <utility>

namespace wind::client {
namespace {

// 키 동작 -> 화면 입력 (쿠션 메뉴처럼 E와 공격 키도 "확인")
constexpr std::array<std::pair<game::Action, game::BagInput>, 9> kKeys{{
    {game::Action::MoveUp, game::BagInput::Up},
    {game::Action::MoveDown, game::BagInput::Down},
    {game::Action::MoveLeft, game::BagInput::Left},
    {game::Action::MoveRight, game::BagInput::Right},
    {game::Action::Confirm, game::BagInput::Confirm},
    {game::Action::Interact, game::BagInput::Confirm},
    {game::Action::Attack, game::BagInput::Confirm},
    {game::Action::SortBag, game::BagInput::Sort},
    {game::Action::Quit, game::BagInput::Back},
}};

}  // namespace

ItemHud::ItemHud(ItemSetup& items, engine::TextureHandle icons) : items_(items), icons_(icons), view_(icons) {
    screen_.set_use_available(true);  // 물약 마시기가 생겼어요 (14d): "사용"을 고를 수 있어요
}

bool ItemHud::handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                           game::FieldState& field, bool can_open) {
    // I: 열려 있으면 닫고, 닫혀 있으면 (열 수 있을 때) 열어요. 이번 장의 다른 키는 보지 않아요
    if (bindings.pressed(game::Action::ToggleBag, input)) {
        if (screen_.is_open()) {
            screen_.close();
        } else if (can_open) {
            screen_.open();
        }
        return false;
    }
    if (!screen_.is_open()) return false;
    bool used_quit = false;
    for (const auto& [action, bag_input] : kKeys) {
        if (!screen_.is_open() || !bindings.pressed(action, input)) continue;  // 한 장에 여러 키도 차례로
        if (action == game::Action::Quit) used_quit = true;                    // Esc는 게임을 끄지 않아요
        const auto command = screen_.handle(bag_input, items_.bag, items_.catalog);
        switch (command.kind) {
            case game::BagCommand::Kind::Drop: field.request_put_down(command.slot); break;  // 다음 박자에 내려놓기
            case game::BagCommand::Kind::Sort: field.request_sort(); break;                   // 다음 박자에 정렬
            case game::BagCommand::Kind::Use:  // 물약 마시기 (14d): 다음 박자에 마시고, 화면은 닫혀요 (개발자와 정함)
                field.request_use(command.slot);
                screen_.close();
                break;
            case game::BagCommand::Kind::Close:
            case game::BagCommand::Kind::None: break;
        }
    }
    return used_quit;
}

void ItemHud::update(float dt_seconds, game::FieldState& field, bool can_open) {
    for (const auto& notice : field.take_loot_notices()) toasts_.push(notice, items_.catalog);  // 주움·가득 참 (14b)
    toasts_.update(dt_seconds);
    if (!can_open && screen_.is_open()) screen_.close();  // 쓰러짐·쉬기·맵 옮기기·쿠션 메뉴면 닫혀요
    view_.update(dt_seconds, screen_.is_open());
}

void ItemHud::draw(engine::UiPainter& painter, engine::Vec2 screen) const {
    draw_loot_toasts(painter, icons_, screen, toasts_);
    view_.draw(painter, screen, screen_, items_.bag, items_.catalog);
}

}  // namespace wind::client
