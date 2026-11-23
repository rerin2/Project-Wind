#pragma once

// 이 파일은 무엇을 하나요?
// 필드 위에 얹는 아이템 UI를 한데 모아 게임 루프에 이어요. (14b 알림 줄, 14c 인벤토리 화면)
// - 왼쪽 아래 알림 줄(LootToasts): 필드가 모아 둔 줍기·가득 참 알림을 가져와 보여 줘요.
// - 인벤토리 화면(BagScreen + BagScreenView): I로 열고 닫아요. 열려 있으면 방향키·Enter·E·R·Esc를 화면이 써요.
//   "사용"(물약, 14d)·"버리기"·"정렬"은 필드에 요청하고, 필드가 다음 박자에 규칙대로 처리해요 (UI는 가방을 직접 바꾸지 않아요).
//   "사용"을 고르면 화면이 닫혀서 마시는 모습이 보여요 (개발자와 정함).
// - 쓰러지거나, 쉬거나, 맵을 옮기거나, 쿠션 메뉴가 열리면 화면이 저절로 닫혀요 (can_open = false).
// 열려 있는 동안 게임은 계속 흐르고, 게임 루프가 주인공 입력을 막아 주인공은 제자리에 서 있어요 (개발자와 정함).

#include "app/bag_screen_view.hpp"
#include "engine/input/input_state.hpp"
#include "game/input/input_bindings.hpp"
#include "game/ui/bag_screen.hpp"
#include "game/ui/loot_toasts.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class FieldState;
}

namespace wind::client {

struct ItemSetup;

class ItemHud {
public:
    // items: 도감·가방 (앱이 가지고 있어요), icons: 아이템 아이콘 그림 판
    ItemHud(ItemSetup& items, engine::TextureHandle icons);

    // 키 확인 (필드 입력보다 먼저). can_open: 지금 인벤토리를 열 수 있나.
    // Esc를 화면이 썼으면 true (게임을 끄지 않아요)
    bool handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                      game::FieldState& field, bool can_open);
    // 인벤토리 화면이 열려 있나요? (게임 루프가 주인공 입력을 막아요)
    [[nodiscard]] bool bag_open() const { return screen_.is_open(); }
    // 화면 한 장마다: 필드의 알림을 가져오고, 열 수 없게 됐으면 화면을 닫고, 시계를 돌려요
    void update(float dt_seconds, game::FieldState& field, bool can_open);
    // 알림 줄과 인벤토리 화면
    void draw(engine::UiPainter& painter, engine::Vec2 screen) const;

private:
    ItemSetup& items_;
    engine::TextureHandle icons_;
    game::LootToasts toasts_;   // 왼쪽 아래 알림 줄 (14b)
    game::BagScreen screen_;    // 인벤토리 화면 생각 (14c)
    BagScreenView view_;        // 인벤토리 화면 그리기 (14c)
};

}  // namespace wind::client
