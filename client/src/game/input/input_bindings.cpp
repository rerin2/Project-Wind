// 이 파일은 무엇을 하나요?
// 리모컨 버튼표(키 바인딩)를 실제로 만들고 확인하는 곳이에요.

#include "game/input/input_bindings.hpp"

#include <algorithm>

namespace wind::game {
namespace {

using engine::Key;

// 동작(Action)을 서랍장 칸 번호로 바꿔요.
constexpr std::size_t index(Action action) {
    return static_cast<std::size_t>(action);
}

}  // namespace

// 처음 게임을 켰을 때 쓰는 기본 버튼표
InputBindings InputBindings::defaults() {
    InputBindings b;
    b.bind(Action::MoveUp, Key::Up);
    b.bind(Action::MoveUp, Key::W);
    b.bind(Action::MoveDown, Key::Down);
    b.bind(Action::MoveDown, Key::S);
    b.bind(Action::MoveLeft, Key::Left);
    b.bind(Action::MoveLeft, Key::A);
    b.bind(Action::MoveRight, Key::Right);
    b.bind(Action::MoveRight, Key::D);
    b.bind(Action::Dodge, Key::Space);
    b.bind(Action::Dodge, Key::K);
    b.bind(Action::Dodge, Key::X);
    b.bind(Action::Run, Key::Shift);
    b.bind(Action::ToggleDebug, Key::F3);
    b.bind(Action::ToggleCollision, Key::F4);
    b.bind(Action::Screenshot, Key::F12);
    b.bind(Action::SkipTime, Key::F5);
    b.bind(Action::NextWeather, Key::F6);
    b.bind(Action::ToggleUiTest, Key::F7);
    b.bind(Action::Confirm, Key::Enter);
    b.bind(Action::Quit, Key::Escape);
    return b;
}

void InputBindings::bind(Action action, engine::Key key) {
    auto& keys = keys_[index(action)];
    if (std::ranges::find(keys, key) == keys.end()) {
        keys.push_back(key);  // 같은 키를 두 번 넣지 않는다
    }
}

void InputBindings::clear(Action action) {
    keys_[index(action)].clear();
}

// 연결된 키를 하나씩 보면서 "누르고 있는 키가 하나라도 있나?" 확인해요.
bool InputBindings::held(Action action, const engine::InputState& input) const {
    return std::ranges::any_of(keys_[index(action)], [&](Key k) { return input.held(k); });
}

// 연결된 키 중 "방금 누른 키가 하나라도 있나?" 확인해요.
bool InputBindings::pressed(Action action, const engine::InputState& input) const {
    return std::ranges::any_of(keys_[index(action)], [&](Key k) { return input.pressed(k); });
}

}  // namespace wind::game
