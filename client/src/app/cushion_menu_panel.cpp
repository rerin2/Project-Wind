// 이 파일은 무엇을 하나요?
// 쿠션 메뉴: 키 -> 메뉴 입력, 결과 -> 필드 (일어나기, 레벨업 적용과 연출).

#include "app/cushion_menu_panel.hpp"

#include "app/field_ui.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/states/field_state.hpp"

#include <array>
#include <format>
#include <utility>

namespace wind::client {
namespace {

// 키 동작 -> 메뉴 입력 (E와 공격 키도 "확인"으로 받아요: 쿠션 앞에서 E를 누르는 손 그대로)
constexpr std::array<std::pair<game::Action, game::MenuInput>, 8> kKeys{{
    {game::Action::MoveUp, game::MenuInput::Up},
    {game::Action::MoveDown, game::MenuInput::Down},
    {game::Action::MoveLeft, game::MenuInput::Left},
    {game::Action::MoveRight, game::MenuInput::Right},
    {game::Action::Confirm, game::MenuInput::Confirm},
    {game::Action::Interact, game::MenuInput::Confirm},
    {game::Action::Attack, game::MenuInput::Confirm},
    {game::Action::Back, game::MenuInput::Back},
}};

}  // namespace

bool CushionMenuPanel::handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                                    game::HeroProgress& progress, game::FieldState& field, FieldUi& ui) {
    bool used_quit = false;
    for (const auto& [action, menu_input] : kKeys) {
        if (!menu_.is_open() || !bindings.pressed(action, input)) continue;  // 한 장에 여러 키도 차례로
        if (action == game::Action::Back) used_quit = true;                  // Esc는 게임을 끄지 않아요
        switch (menu_.handle(menu_input, progress)) {
            case game::MenuResult::Leave:
                (void)field.stand_up();  // 기지개 켜고 일어나요
                break;
            case game::MenuResult::LeveledUp:
                field.refresh_hero_stats();  // 최대 HP·MP·스태미나와 공격 보너스를 바로 (바가 늘어나요)
                ui.announce(std::format("레벨 {}", progress.level()));  // 레벨업 배너 (명세 8.3)
                break;
            case game::MenuResult::Denied:  // 화면이 비용 줄을 흔들어요 (CushionMenuView)
            case game::MenuResult::None:
                break;
        }
    }
    return used_quit;
}

}  // namespace wind::client
