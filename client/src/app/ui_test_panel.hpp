#pragma once

// 이 파일은 무엇을 하나요?
// "UI 시험판"이에요 (F7, 개발자용). 단계 8에서 만든 UI 부품을 한 화면에서 시험해요.
// - 창(9조각 테두리), 제목(굵은 글꼴), 줄바꿈되는 설명 글, 잘리는 글
// - 버튼 세 개와 입력 포커스: 위·아래로 고르고 Enter로 눌러요
// - 열려 있는 동안 게임 입력은 막혀요 (캐릭터가 안 움직여요). Esc나 F7로 닫아요.
// 버튼을 누르면 할 일을 돌려주고, 실제 일(시간 넘기기 등)은 게임 루프가 해요.

#include "engine/input/input_state.hpp"
#include "engine/ui/focus_list.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/input/input_bindings.hpp"

#include <cstdint>

namespace wind::client {

// 시험판 버튼을 눌렀을 때 할 일
enum class UiCommand : std::uint8_t { None, SkipTime, NextWeather };

class UiTestPanel {
public:
    UiTestPanel();

    void toggle();
    [[nodiscard]] bool open() const { return open_; }

    // 열려 있을 때만 키를 봐요. Esc(Quit)를 누르면 게임을 끄지 않고 시험판만 닫아요.
    // consumed_quit: Esc를 시험판이 썼으면 true
    UiCommand handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                           bool& consumed_quit);
    void update(float dt_seconds);
    void draw(engine::UiPainter& painter, engine::Vec2 screen);

private:
    engine::FocusList focus_;
    bool open_ = false;
    float appear_ = 0.0f;  // 열릴 때 0 -> 1 (톡 튀어나오는 크기 변화)
};

}  // namespace wind::client
