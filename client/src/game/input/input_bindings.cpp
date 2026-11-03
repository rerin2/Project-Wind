// 이 파일은 무엇을 하나요?
// 리모컨 버튼표(키 바인딩)와 "어느 방향으로 갈까?"를 실제로 계산하는 곳이에요.

#include "game/input/input_bindings.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {
namespace {

using engine::Key;
using world::Direction;

// 동작(Action)을 서랍장 칸 번호로 바꿔요.
constexpr std::size_t index(Action action) {
    return static_cast<std::size_t>(action);
}

// 방향과 이동 동작 대응표 (위 방향 = "위로 가기" 동작 ...)
constexpr std::array<std::pair<Direction, Action>, 4> kMoveActions{{
    {Direction::Up, Action::MoveUp},
    {Direction::Down, Action::MoveDown},
    {Direction::Left, Action::MoveLeft},
    {Direction::Right, Action::MoveRight},
}};

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
    b.bind(Action::ToggleDebug, Key::F3);
    b.bind(Action::ToggleCollision, Key::F4);
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

void MovementIntent::update(const InputBindings& bindings, const engine::InputState& input) {
    // 이번 프레임에 "새로" 누른 방향이 있으면 그걸 마지막 방향으로 기억해요.
    for (const auto& [dir, action] : kMoveActions) {
        if (bindings.pressed(action, input)) {
            last_pressed_ = dir;
        }
    }
}

std::optional<world::Direction> MovementIntent::direction(const InputBindings& bindings,
                                                          const engine::InputState& input) const {
    // 마지막으로 누른 방향이 아직 눌려 있으면 그 방향
    if (last_pressed_) {
        const auto it = std::ranges::find(kMoveActions, *last_pressed_, &std::pair<Direction, Action>::first);
        if (bindings.held(it->second, input)) {
            return last_pressed_;
        }
    }
    // 아니면 눌려 있는 방향 중 표 순서상 첫 번째 (위 → 아래 → 왼쪽 → 오른쪽)
    for (const auto& [dir, action] : kMoveActions) {
        if (bindings.held(action, input)) {
            return dir;
        }
    }
    return std::nullopt;  // 방향키를 아무것도 안 누르고 있어요
}

}  // namespace wind::game
