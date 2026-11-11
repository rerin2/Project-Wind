// 이 파일은 무엇을 하나요?
// 시험용 가짜 키 누르기(--auto-move, --press)를 키보드 상태에 적어 넣는 곳이에요.

#include "app/scripted_input.hpp"

namespace wind::client {

// 방향 -> 화살표 키
engine::Key move_key(world::Direction dir) {
    switch (dir) {
        case world::Direction::Up: return engine::Key::Up;
        case world::Direction::Down: return engine::Key::Down;
        case world::Direction::Left: return engine::Key::Left;
        case world::Direction::Right: return engine::Key::Right;
    }
    return engine::Key::Down;
}

// 시험용 --press 키 이름을 엔진 키로 바꿔요. (이름은 실행 옵션을 읽을 때 이미 검사했어요)
engine::Key scripted_key(std::string_view name) {
    if (name == "f3") return engine::Key::F3;
    if (name == "f4") return engine::Key::F4;
    if (name == "f5") return engine::Key::F5;
    if (name == "f6") return engine::Key::F6;
    if (name == "f7") return engine::Key::F7;
    if (name == "enter") return engine::Key::Enter;
    if (name == "esc") return engine::Key::Escape;
    if (name == "f12") return engine::Key::F12;
    if (name == "space") return engine::Key::Space;
    if (name == "shift") return engine::Key::Shift;
    if (name == "j") return engine::Key::J;
    if (name == "up") return engine::Key::Up;
    if (name == "down") return engine::Key::Down;
    if (name == "left") return engine::Key::Left;
    if (name == "right") return engine::Key::Right;
    return engine::Key::Unknown;
}

// frame번째 장: 그 장에 누르기로 한 키는 누르고, 바로 전 장에 눌렀던 키는 떼요.
void apply_scripted_presses(const std::vector<ClientOptions::KeyPress>& presses, std::uint64_t frame,
                            engine::InputState& input) {
    for (const auto& press : presses) {
        if (press.frame == frame) {
            input.set_key(scripted_key(press.key), true);
        } else if (press.frame + 1 == frame) {
            input.set_key(scripted_key(press.key), false);
        }
    }
}

// frame번째 장: 누르고 있는 구간(from~to) 안이면 누르고, 구간이 막 끝났으면 떼요.
void apply_scripted_holds(const std::vector<ClientOptions::KeyHold>& holds, std::uint64_t frame,
                          engine::InputState& input) {
    for (const auto& hold : holds) {
        if (frame >= hold.from && frame <= hold.to) {
            input.set_key(scripted_key(hold.key), true);
        } else if (frame == hold.to + 1) {
            input.set_key(scripted_key(hold.key), false);
        }
    }
}

}  // namespace wind::client
