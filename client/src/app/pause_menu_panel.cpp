// 이 파일은 무엇을 하나요?
// 일시정지 메뉴 열기·닫기, 키를 메뉴 입력으로 바꾸기, 가방 명령을 필드에 요청하기.

#include "app/pause_menu_panel.hpp"

#include "app/item_setup.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/states/field_state.hpp"

#include "wind/items/equip_slot.hpp"
#include "wind/log.hpp"

#include <array>
#include <utility>

namespace wind::client {
namespace {

// 키 동작 -> 메뉴 입력 (쿠션 메뉴처럼 E와 공격 키도 "확인")
constexpr std::array<std::pair<game::Action, game::PauseInput>, 10> kKeys{{
    {game::Action::MoveUp, game::PauseInput::Up},
    {game::Action::MoveDown, game::PauseInput::Down},
    {game::Action::MoveLeft, game::PauseInput::Left},
    {game::Action::MoveRight, game::PauseInput::Right},
    {game::Action::Confirm, game::PauseInput::Confirm},
    {game::Action::Interact, game::PauseInput::Confirm},
    {game::Action::Attack, game::PauseInput::Confirm},
    {game::Action::SortBag, game::PauseInput::Sort},
    {game::Action::Back, game::PauseInput::Back},
    {game::Action::NextTab, game::PauseInput::NextTab},
}};

}  // namespace

PauseMenuPanel::PauseMenuPanel(ItemSetup& items, const game::HeroProgress& progress, engine::TextureHandle icons,
                               engine::TextureHandle spell_icons)
    : items_(items), progress_(progress), view_(icons, spell_icons) {
    menu_.set_use_available(true);  // 물약 마시기 (14d): "사용"을 고를 수 있어요
    view_.set_bag_info(&items_.weapons, &progress_, &items_.quick);  // 무기 무게 줄(15a), 단축 칸 번호(15f)
}

bool PauseMenuPanel::handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                                  game::FieldState& field, bool can_open) {
    if (menu_.is_open()) return handle_open(input, bindings, field);
    if (!can_open) return false;
    if (bindings.pressed(game::Action::ToggleBag, input)) {
        menu_.open(game::MenuTab::Bag);  // I: 바로 가방 탭으로
    } else if (bindings.pressed(game::Action::Menu, input)) {
        menu_.open();  // Esc / Tab: 마지막으로 보던 탭으로 (이번 장의 Tab은 탭을 넘기지 않아요)
    }
    return false;
}

bool PauseMenuPanel::handle_open(const engine::InputState& input, const game::InputBindings& bindings,
                                 game::FieldState& field) {
    if (bindings.pressed(game::Action::ToggleBag, input)) {  // I: 가방 탭이면 닫고, 아니면 가방 탭으로
        if (menu_.tab() == game::MenuTab::Bag) {
            menu_.close();
        } else {
            menu_.open(game::MenuTab::Bag);
        }
        return false;
    }
    register_quick_slots(input, bindings);  // 가방 탭에서 1~4: 커서 아이템을 단축 칸에 (15f)
    const std::size_t spell_rows = progress_.spells().learned().size();
    for (const auto& [action, menu_input] : kKeys) {
        if (!menu_.is_open() || !bindings.pressed(action, input)) continue;  // 한 장에 여러 키도 차례로
        const auto command = menu_.handle(menu_input, items_.bag, items_.catalog, items::kEquipSlotCount, spell_rows);
        if (command.kind == game::PauseCommand::Kind::Quit) return true;  // 게임 끄기
        if (command.kind != game::PauseCommand::Kind::Bag) continue;
        switch (command.bag.kind) {
            case game::BagCommand::Kind::Drop: field.request_put_down(command.bag.slot); break;  // 다음 박자에 내려놓기
            case game::BagCommand::Kind::Sort: field.request_sort(); break;                       // 다음 박자에 정렬
            case game::BagCommand::Kind::Use:  // 물약 마시기 (14d): 다음 박자에 마시고, 메뉴는 닫혀요
                field.request_use(command.bag.slot);
                menu_.close();
                break;
            case game::BagCommand::Kind::Equip:  // 무기 장착 (15a): 다음 박자에 바꿔 끼고, 메뉴는 닫혀요
                field.request_equip(command.bag.slot);
                menu_.close();
                break;
            case game::BagCommand::Kind::Close:
            case game::BagCommand::Kind::None: break;
        }
    }
    return false;
}

// 가방 탭에서 작은 메뉴가 닫혀 있을 때만 (명세 15단계 메모: 아이템에 커서를 두고 1~4).
// 쓸 수 있는 아이템만 등록되고, 같은 칸에 한 번 더 누르면 풀려요 (QuickSlots 규칙). 빈 칸이나 재료는 아무 일도 없어요.
void PauseMenuPanel::register_quick_slots(const engine::InputState& input, const game::InputBindings& bindings) {
    if (menu_.tab() != game::MenuTab::Bag || menu_.bag_screen().menu_open()) return;
    const auto& stack = items_.bag.at(menu_.bag_screen().cursor());
    if (!stack) return;  // 빈 칸
    for (std::size_t i = 0; i < game::kQuickSlotActions.size(); ++i) {
        if (!bindings.pressed(game::kQuickSlotActions[i], input)) continue;
        const auto result = items_.quick.assign(i, stack->item_id, items_.catalog);
        log::debug("Quick slot {} <- {}: {}", i + 1, stack->item_id, static_cast<int>(result));
    }
}

void PauseMenuPanel::update(float dt_seconds, bool can_open) {
    if (!can_open && menu_.is_open()) menu_.close();  // 쓰러짐·쉬기·맵 옮기기·쿠션 메뉴면 닫혀요
    view_.update(dt_seconds, menu_);
}

void PauseMenuPanel::draw(engine::UiPainter& painter, engine::Vec2 screen) const {
    view_.draw(painter, screen, menu_, items_, progress_);
}

}  // namespace wind::client
