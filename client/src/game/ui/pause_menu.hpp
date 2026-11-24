#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴의 "생각하는 부분"이에요. (명세 13.1, 13.4, 15e) 그리기는 app 층(pause_menu_view)이 해요.
// - 탭 다섯 개: 가방 · 장비 · 마술 · 능력치 · 시스템. Tab으로 다음 탭 (끝에서 처음으로).
// - 가방 탭은 원래 인벤토리 화면(BagScreen)을 그대로 안에 품어요. 키도 그대로 넘겨요.
// - 장비·마술 탭은 보기만 해요 (개발자와 정함): 위·아래로 칸·마술을 골라 설명을 봐요.
//   장착은 가방 탭에서(15a), 마술 칸 바꾸기는 쿠션에서(15f).
// - 능력치 탭은 보기만 하고 고를 것이 없어요.
// - 시스템 탭: 설정(흐림, 17단계) · 타이틀로(흐림, 17단계) · 게임 끄기 (개발자와 정함).
// - Esc(뒤로): 가방의 작은 메뉴가 열려 있으면 그것만 닫고, 아니면 메뉴를 닫아요.
// - 열려 있는 동안 게임은 계속 흐르고, 게임 루프가 주인공 입력을 막아 주인공은 제자리에 서요 (개발자와 정함).
// 키 대신 PauseInput을 받아서 창 없이 테스트할 수 있어요.

#include "game/ui/bag_screen.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace wind::game {

// 메뉴 탭 (순서 = 화면 왼쪽부터)
enum class MenuTab : std::uint8_t { Bag, Gear, Spells, Stats, System };
inline constexpr std::size_t kMenuTabCount = 5;

// 시스템 탭 항목
enum class SystemItem : std::uint8_t { Settings, Title, Quit };
inline constexpr std::size_t kSystemItemCount = 3;

// 메뉴가 받는 입력 (앱이 키를 이것으로 바꿔 줘요)
enum class PauseInput : std::uint8_t { Up, Down, Left, Right, Confirm, Back, NextTab, Sort };

// 입력의 결과로 앱이 할 일
struct PauseCommand {
    enum class Kind : std::uint8_t {
        None,   // 메뉴 안에서만 바뀌었어요
        Close,  // 메뉴가 닫혔어요
        Quit,   // 게임을 꺼 주세요 (시스템 탭 "게임 끄기")
        Bag,    // 가방 탭의 명령 (bag에 들어 있어요: 사용·장착·버리기·정렬)
    };
    Kind kind = Kind::None;
    BagCommand bag;  // kind가 Bag일 때
};

class PauseMenu {
public:
    // 메뉴를 이 탭으로 열어요 (탭마다 커서는 지난번 자리 그대로)
    void open(MenuTab tab);
    // 마지막으로 보던 탭으로 열어요 (Esc / Tab)
    void open() { open(tab_); }
    void close();
    [[nodiscard]] bool is_open() const { return open_; }
    // 메뉴가 열려 있으면 게임 입력을 막아요 (명세 13.3 입력 포커스)
    [[nodiscard]] bool captures_input() const { return open_; }

    // 입력 하나를 처리해요. bag·catalog는 가방 탭이 칸을 보는 데만 써요.
    // gear_rows: 장비 칸 수, spell_rows: 배운 마술 수 (위·아래로 고를 줄 수)
    PauseCommand handle(PauseInput input, const items::Inventory& bag, const items::ItemCatalog& catalog,
                        std::size_t gear_rows, std::size_t spell_rows);

    [[nodiscard]] MenuTab tab() const { return tab_; }
    [[nodiscard]] const BagScreen& bag_screen() const { return bag_; }
    // 가방의 "사용"을 고를 수 있게 해요 (14d부터 늘 켜져 있어요)
    void set_use_available(bool available) { bag_.set_use_available(available); }
    // 장비·마술 탭에서 고른 줄, 시스템 탭에서 고른 항목
    [[nodiscard]] std::size_t gear_row() const { return gear_row_; }
    [[nodiscard]] std::size_t spell_row() const { return spell_row_; }
    [[nodiscard]] SystemItem system_item() const { return system_item_; }
    // 이 시스템 항목을 고를 수 있나요? (설정·타이틀로는 17단계까지 흐림)
    [[nodiscard]] static bool system_enabled(SystemItem item) { return item == SystemItem::Quit; }

private:
    // 위·아래로 줄을 옮겨요 (끝에서 멈춰요, 줄이 없으면 0)
    static void move_row(std::size_t& row, PauseInput input, std::size_t rows);
    PauseCommand handle_system(PauseInput input);

    bool open_ = false;
    MenuTab tab_ = MenuTab::Bag;
    BagScreen bag_;
    std::size_t gear_row_ = 0;
    std::size_t spell_row_ = 0;
    SystemItem system_item_ = SystemItem::Quit;  // 고를 수 있는 항목에서 시작
};

}  // namespace wind::game
