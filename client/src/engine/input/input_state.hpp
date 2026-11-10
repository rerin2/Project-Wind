#pragma once

// 이 파일은 무엇을 하나요?
// 키보드의 어떤 키가 눌렸는지 적어 두는 "출석부"예요. (명세 5.8)
// 플랫폼 계층(운영체제와 이야기하는 부분)이 키 소식을 받아서 여기에 적고,
// 게임은 출석부만 보고 "이 키 지금 눌려 있니?"라고 물어봐요.
// SDL(창과 키보드를 다루는 도구 상자)의 키 번호는 이 파일 밖으로 나가지 않아요.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace wind::engine {

// 엔진이 알아듣는 키 목록. 새 키가 필요하면 여기에 더해요.
enum class Key : std::uint8_t {
    Unknown,  // 모르는 키
    Up,
    Down,
    Left,
    Right,
    W,
    A,
    S,
    D,
    Escape,
    Enter,
    Space,
    K,
    X,
    F3,
    F4,
    F5,
    F6,
    F12,
    Count  // 키가 몇 개인지 세는 용도 (진짜 키가 아니에요)
};

// 키 이름을 글자로 바꿔요. (디버그 화면에 보여 줄 때 써요)
[[nodiscard]] std::string_view to_string(Key key);

// 키보드 출석부
class InputState {
public:
    // 화면 한 장을 그리기 시작할 때마다 불러요.
    // "방금 눌렀음/방금 뗐음" 표시는 한 장 동안만 유효해서 지워요.
    void begin_frame();

    // 키가 눌렸는지(down = true) 떼어졌는지(down = false) 출석부에 적어요.
    // 키를 꾹 누르고 있을 때 컴퓨터가 계속 보내는 "또 눌렸어요" 신호는 적지 않아요.
    void set_key(Key key, bool down);

    // 다른 창을 클릭해서 우리 게임 창에서 눈을 떼면 모든 키를 뗀 것으로 해요.
    // 안 그러면 키를 뗀 소식을 못 받아서 캐릭터가 계속 걸어갈 수 있어요.
    void release_all();

    // 채팅 입력창 같은 UI(화면 위 버튼·창)가 키보드를 쓰고 있으면,
    // 게임은 키가 안 눌린 것으로 봐요. 채팅하다가 캐릭터가 움직이면 안 되니까요.
    void set_ui_capture(bool captured) { ui_capture_ = captured; }
    [[nodiscard]] bool ui_captured() const { return ui_capture_; }

    // 지금 누르고 있나요?
    [[nodiscard]] bool held(Key key) const;
    // 이번 장에서 새로 눌렀나요? (버튼을 "딸깍" 한 번 누른 순간)
    [[nodiscard]] bool pressed(Key key) const;
    // 이번 장에서 손을 뗐나요?
    [[nodiscard]] bool released(Key key) const;

private:
    static constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);
    // 키를 출석부의 몇 번째 줄인지(번호)로 바꿔요.
    static std::size_t index(Key key) { return static_cast<std::size_t>(key); }

    std::array<bool, kKeyCount> held_{};      // 누르고 있는 키
    std::array<bool, kKeyCount> pressed_{};   // 이번 장에 새로 누른 키
    std::array<bool, kKeyCount> released_{};  // 이번 장에 뗀 키
    bool ui_capture_ = false;                 // UI가 키보드를 쓰고 있는지
};

}  // namespace wind::engine
