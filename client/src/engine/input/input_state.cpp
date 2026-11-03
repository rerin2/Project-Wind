// 이 파일은 무엇을 하나요?
// 키보드 출석부(InputState)에 적고, 읽는 방법을 실제로 적은 곳이에요.

#include "engine/input/input_state.hpp"

namespace wind::engine {

// 키마다 사람이 읽을 이름을 붙여요.
std::string_view to_string(Key key) {
    switch (key) {
        case Key::Up: return "Up";
        case Key::Down: return "Down";
        case Key::Left: return "Left";
        case Key::Right: return "Right";
        case Key::W: return "W";
        case Key::A: return "A";
        case Key::S: return "S";
        case Key::D: return "D";
        case Key::Escape: return "Esc";
        case Key::Enter: return "Enter";
        case Key::F3: return "F3";
        case Key::F4: return "F4";
        case Key::Unknown:
        case Key::Count: break;
    }
    return "Unknown";
}

// 새 장이 시작되면 "방금 눌렀음/뗐음" 표시만 지워요. "누르고 있음"은 그대로 둬요.
void InputState::begin_frame() {
    pressed_.fill(false);
    released_.fill(false);
}

void InputState::set_key(Key key, bool down) {
    if (key == Key::Unknown || key == Key::Count) {
        return;  // 모르는 키는 무시해요
    }
    const auto i = index(key);
    // 안 눌려 있다가 눌리면 "방금 눌렀음", 눌려 있다가 떼면 "방금 뗐음"
    if (down && !held_[i]) {
        pressed_[i] = true;
    } else if (!down && held_[i]) {
        released_[i] = true;
    }
    held_[i] = down;
}

// 누르고 있던 키를 전부 "뗐음"으로 바꿔요.
void InputState::release_all() {
    for (std::size_t i = 0; i < kKeyCount; ++i) {
        if (held_[i]) {
            released_[i] = true;
        }
        held_[i] = false;
    }
}

// 아래 세 함수는 모두 UI가 키보드를 쓰고 있으면 무조건 false를 돌려줘요.
bool InputState::held(Key key) const {
    return !ui_capture_ && key != Key::Count && held_[index(key)];
}

bool InputState::pressed(Key key) const {
    return !ui_capture_ && key != Key::Count && pressed_[index(key)];
}

bool InputState::released(Key key) const {
    return !ui_capture_ && key != Key::Count && released_[index(key)];
}

}  // namespace wind::engine
