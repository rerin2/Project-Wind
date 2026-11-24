#pragma once

// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 메뉴의 "생각하는 부분"이에요. (명세 8.7 쿠션 메뉴, 13.1, 13c) 그리기는 app 층(cushion_menu_view)이 해요.
// - 메뉴 항목: 레벨업 / 마술 칸(15f) / 저장(16단계, 흐리게) / 떠나기
// - 레벨업을 고르면 레벨업 화면(LevelUpSheet)으로 들어가요. 뒤로(Esc)면 메뉴로 돌아와요.
// - 마술 칸을 고르면 마술 칸 화면(SpellSlotSheet)으로 들어가요 (15f). 뒤로면 메뉴로 돌아와요.
// - 메뉴에서 뒤로(Esc)나 "떠나기"면 메뉴가 닫히고 주인공이 일어나요.
// - 열려 있는 동안 게임 시간은 멈춰요 (게임 루프가 박자를 돌리지 않아요, 명세 8.7).
// 키 대신 MenuInput(위·아래·왼쪽·오른쪽·확인·뒤로)을 받아서, 창 없이 테스트할 수 있어요.

#include "engine/ui/focus_list.hpp"
#include "game/ui/level_up_sheet.hpp"
#include "game/ui/menu_input.hpp"
#include "game/ui/spell_slot_sheet.hpp"

#include <cstdint>

namespace wind::game {

class HeroProgress;

class SpellShelf;

// 입력의 결과로 게임 루프가 할 일
enum class MenuResult : std::uint8_t {
    None,           // 메뉴 안에서만 바뀌었어요
    Leave,          // 메뉴가 닫혔어요: 주인공을 일으켜요
    LeveledUp,      // 레벨업을 확정했어요: 새 능력치를 필드에 적용하고 연출을 틀어요
    Denied,         // 확정하려 했지만 안 됐어요 (룬이 모자람, 지능이 모자람): 흔들림 같은 짧은 알림
    SpellsChanged,  // 마술 칸이 바뀌었어요 (15f): 주인공 손의 마술을 다시 쥐여 줘요
};

class CushionMenu {
public:
    enum class Item : std::uint8_t { LevelUp, SpellSlots, Save, Leave };
    enum class Screen : std::uint8_t { Closed, Main, LevelUp, SpellSlots };
    static constexpr std::size_t kItemCount = 4;

    CushionMenu();

    // 마술 선반을 알려 줘요 (15f): 요구 지능을 보는 데 써요. 알려 주면 "마술 칸" 항목이 켜져요.
    void set_spell_shelf(const SpellShelf& shelf);

    // 메뉴를 열어요 (쿠션에서 쉬기 이동이 끝난 직후)
    void open();
    [[nodiscard]] bool is_open() const { return screen_ != Screen::Closed; }
    [[nodiscard]] Screen screen() const { return screen_; }
    // 입력 하나를 처리해요
    MenuResult handle(MenuInput input, HeroProgress& progress);

    [[nodiscard]] Item focused_item() const { return static_cast<Item>(focus_.current()); }
    [[nodiscard]] bool item_enabled(Item item) const { return focus_.enabled(static_cast<std::size_t>(item)); }
    [[nodiscard]] const LevelUpSheet& sheet() const { return sheet_; }
    [[nodiscard]] const SpellSlotSheet& slot_sheet() const { return slot_sheet_; }
    // 확정이 거절된 횟수 (늘기만 해요. 화면이 바뀐 걸 보고 비용 줄을 흔들어요)
    [[nodiscard]] unsigned denied_count() const { return denied_count_; }

private:
    MenuResult handle_main(MenuInput input, HeroProgress& progress);
    MenuResult handle_level_up(MenuInput input, HeroProgress& progress);
    MenuResult handle_spell_slots(MenuInput input, HeroProgress& progress);

    engine::FocusList focus_;
    LevelUpSheet sheet_;
    SpellSlotSheet slot_sheet_;            // 마술 칸 화면 (15f)
    const SpellShelf* shelf_ = nullptr;    // 요구 지능을 보는 마술 선반 (없으면 "마술 칸"이 흐려요)
    Screen screen_ = Screen::Closed;
    unsigned denied_count_ = 0;
};

}  // namespace wind::game
