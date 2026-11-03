#pragma once

// 이 파일은 무엇을 하나요?
// 키 바인딩(리모컨 버튼마다 기능을 정해 두는 것)을 맡아요.
// 예: "위로 가기" 기능 = 위 방향키 또는 W 키
// 게임은 "W 키가 눌렸니?"가 아니라 "위로 가기 버튼이 눌렸니?"라고 물어봐요.
// 그래서 나중에 키를 바꿔도 게임 코드는 그대로 둘 수 있어요. (명세 5.8)
// 바꾼 키를 저장하는 기능은 나중 단계에서 만들어요.

#include "engine/input/input_state.hpp"

#include "wind/world/direction.hpp"

#include <array>
#include <optional>
#include <vector>

namespace wind::game {

// 게임에서 할 수 있는 동작(기능) 목록
enum class Action : std::uint8_t {
    MoveUp,           // 위로 가기
    MoveDown,         // 아래로 가기
    MoveLeft,         // 왼쪽으로 가기
    MoveRight,        // 오른쪽으로 가기
    ToggleDebug,      // 디버그 정보 표시 (개발자용 정보 화면 켜기/끄기)
    ToggleCollision,  // 충돌 칸 표시 (못 지나가는 칸을 빨갛게 보여 주기)
    Quit,             // 게임 끄기
    Count  // 개수 표시용 (진짜 동작은 아니고, 동작이 몇 개인지 세는 데 써요)
};

// "동작 ↔ 키" 연결표
class InputBindings {
public:
    // 기본 바인딩: 방향키/WASD 이동, F3 디버그, F4 충돌 표시, Esc 종료
    [[nodiscard]] static InputBindings defaults();

    // 동작에 키를 하나 더 연결해요. (한 동작에 키 여러 개 가능)
    void bind(Action action, engine::Key key);
    // 동작에 연결된 키를 모두 지워요.
    void clear(Action action);

    // 이 동작에 연결된 키 중 하나라도 "지금 누르고 있으면" true
    [[nodiscard]] bool held(Action action, const engine::InputState& input) const;
    // 이 동작에 연결된 키 중 하나라도 "방금 눌렀으면" true
    [[nodiscard]] bool pressed(Action action, const engine::InputState& input) const;

private:
    static constexpr std::size_t kActionCount = static_cast<std::size_t>(Action::Count);
    // 동작 번호마다 연결된 키 목록을 담는 서랍장
    std::array<std::vector<engine::Key>, kActionCount> keys_;
};

// 방향키를 여러 개 같이 누르고 있을 때 어느 쪽으로 갈지 정해요.
// 규칙: "가장 마지막에 누른 키"가 이겨요.
// 예: 위를 누른 채로 오른쪽을 누르면 오른쪽으로, 오른쪽을 떼면 다시 위로 가요.
class MovementIntent {
public:
    // 매 프레임(화면 한 장 그릴 때마다) 불러서 새로 눌린 방향을 기억해요.
    void update(const InputBindings& bindings, const engine::InputState& input);

    // 지금 가고 싶은 방향이에요. 방향키를 하나도 안 누르고 있으면 "없음"(nullopt)
    [[nodiscard]] std::optional<world::Direction> direction(const InputBindings& bindings,
                                                            const engine::InputState& input) const;

private:
    std::optional<world::Direction> last_pressed_;  // 마지막으로 누른 방향
};

}  // namespace wind::game
