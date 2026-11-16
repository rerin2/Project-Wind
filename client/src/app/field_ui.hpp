#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면 위에 얹는 UI를 모아요. (명세 13.2)
// - HUD 스태미나 바 (지연 바, 가득 차면 흐려짐, 지침이면 빨갛게 깜빡임)
// - HUD MP 바 (지연 바, 스태미나 바 바로 아래, 길이는 최대 MP에 비례, 늘 보임)
// - 지역 이름 배너 (새 맵에 들어가면 위에서 내려와 잠깐 보였다가 사라짐)
// - UI 시험판 (F7)
// 게임 규칙은 바꾸지 않고, 필드가 알려 준 값(스태미나 등)을 보여 주기만 해요.

#include "app/ui_test_panel.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/ui/area_banner.hpp"
#include "game/ui/delayed_bar.hpp"

#include <optional>
#include <string_view>

namespace wind::client {

// HUD가 보여 줄 필드 상태
struct HudInfo {
    float stamina = 1.0f;  // 0~1
    bool exhausted = false;
    float mana = 1.0f;       // 0~1
    float mana_max = 50.0f;  // 최대 MP (바 길이가 이 값에 비례해요, 명세 13.2)
    unsigned mana_denied = 0;  // MP가 모자라 마술이 안 나간 횟수. 늘면 MP 바가 깜빡여요 (명세 9.2)
};

class FieldUi {
public:
    FieldUi();

    // 새 지역에 들어왔어요: 이름 배너를 보여 줘요.
    void enter_area(std::string_view display_name);

    // 키 확인. UI가 키를 쓰고 있으면(시험판이 열림) 게임 입력을 막아야 해요.
    UiCommand handle_input(const engine::InputState& input, const game::InputBindings& bindings,
                           bool& consumed_quit);
    [[nodiscard]] bool captures_input() const { return panel_.open(); }

    void update(float dt_seconds, const HudInfo& hud);
    // HUD와 배너 (화면 전환 가림막보다 아래)
    void draw_hud(engine::UiPainter& painter, engine::Vec2 screen);
    // 시험판 (맨 위)
    void draw_panels(engine::UiPainter& painter, engine::Vec2 screen);

private:
    void draw_stamina(engine::UiPainter& painter);
    void draw_mana(engine::UiPainter& painter);
    void draw_banner(engine::UiPainter& painter, engine::Vec2 screen);

    game::AreaBanner banner_;
    game::DelayedBar stamina_;
    game::DelayedBar mana_;
    float mana_max_ = 50.0f;  // 바 길이 계산용
    std::optional<unsigned> mana_denied_seen_;  // 마지막으로 본 "MP 모자람" 횟수 (처음엔 없음)
    float mana_blink_ = 0.0f;  // MP 바가 깜빡일 남은 시간 (초)
    bool exhausted_ = false;
    float clock_ = 0.0f;  // 깜빡임용 시계
    UiTestPanel panel_;
};

}  // namespace wind::client
