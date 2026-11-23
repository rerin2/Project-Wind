#pragma once

// 이 파일은 무엇을 하나요?
// 인벤토리 화면의 "생각하는 부분"이에요. (명세 10.2, 13.1, 14c) 그리기는 app 층(bag_screen_view)이 해요.
// - 24칸을 6칸 × 4줄 격자로 보여 주고, 방향키로 칸을 골라요 (끝에서 멈춰요).
// - 아이템이 있는 칸에서 확인(Enter·E)을 누르면 작은 메뉴: 사용 / 버리기 / 취소
//   "사용"은 소비 아이템이고 쓸 수 있을 때만 (물약 마시기는 14d라 그 전에는 흐려요).
// - 버리기를 고르면 "이 칸을 발밑에 내려놓아 주세요" 명령을 돌려줘요. 정렬(R)도 명령으로 돌려줘요.
//   실제로 가방을 바꾸는 일은 게임 규칙(FieldItems)이 다음 박자에 해요 (전부 성공 또는 전부 실패).
// - 뒤로(Esc)는 작은 메뉴를 닫고, 메뉴가 없으면 화면을 닫아요. I도 화면을 닫아요(앱이 처리).
// - 열려 있는 동안 게임은 계속 흐르고 주인공은 제자리에 서 있어요 (개발자와 정함).
// 키 대신 BagInput을 받아서 창 없이 테스트할 수 있어요.

#include <cstddef>
#include <cstdint>

namespace wind::items {
class Inventory;
class ItemCatalog;
}  // namespace wind::items

namespace wind::game {

// 화면이 받는 입력 (앱이 키를 이것으로 바꿔 줘요)
enum class BagInput : std::uint8_t { Up, Down, Left, Right, Confirm, Back, Sort };

// 입력의 결과로 앱이 할 일
struct BagCommand {
    enum class Kind : std::uint8_t {
        None,   // 화면 안에서만 바뀌었어요
        Close,  // 화면이 닫혔어요
        Use,    // 이 칸의 아이템을 써 주세요 (14d)
        Drop,   // 이 칸의 묶음을 발밑에 내려놓아 주세요
        Sort,   // 가방을 정렬해 주세요
    };
    Kind kind = Kind::None;
    std::size_t slot = 0;  // Use·Drop일 때 어느 칸
};

class BagScreen {
public:
    static constexpr std::size_t kColumns = 6;  // 한 줄 칸 수 (24칸 = 6 × 4)

    // 작은 메뉴의 항목
    enum class Action : std::uint8_t { Use, Drop, Cancel };
    static constexpr std::size_t kActionCount = 3;

    // 화면을 열어요 (커서는 지난번 자리 그대로, 작은 메뉴는 닫힌 채)
    void open();
    void close();
    [[nodiscard]] bool is_open() const { return open_; }
    // 아이템 쓰기를 할 수 있나요? (물약 마시기가 생기는 14d부터 true)
    void set_use_available(bool available) { use_available_ = available; }

    // 입력 하나를 처리해요. bag·catalog는 칸이 비었는지, 쓸 수 있는 아이템인지 보는 데만 써요.
    BagCommand handle(BagInput input, const items::Inventory& bag, const items::ItemCatalog& catalog);

    [[nodiscard]] std::size_t cursor() const { return cursor_; }
    [[nodiscard]] bool menu_open() const { return menu_open_; }
    [[nodiscard]] Action menu_choice() const { return choice_; }
    // 작은 메뉴의 이 항목을 고를 수 있나요? (지금 커서 칸 기준)
    [[nodiscard]] bool action_enabled(Action action, const items::Inventory& bag,
                                      const items::ItemCatalog& catalog) const;

private:
    void move_cursor(BagInput input, std::size_t slot_count);
    BagCommand handle_menu(BagInput input, const items::Inventory& bag, const items::ItemCatalog& catalog);
    // 작은 메뉴에서 step(+1/-1) 쪽으로 다음 고를 수 있는 항목으로 (취소는 늘 고를 수 있어요)
    void move_choice(int step, const items::Inventory& bag, const items::ItemCatalog& catalog);

    bool open_ = false;
    bool menu_open_ = false;
    bool use_available_ = false;
    std::size_t cursor_ = 0;
    Action choice_ = Action::Cancel;
};

}  // namespace wind::game
