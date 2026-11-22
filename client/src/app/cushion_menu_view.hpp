#pragma once

// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 메뉴와 레벨업 화면을 그려요. (명세 8.3, 8.7, 13c)
// 생각(무엇을 골랐나, 비용, 확정)은 game::CushionMenu가 하고, 여기는 보여 주기만 해요.
// - 메뉴: 오른쪽 창에 네 항목(아직 없는 기능은 흐리게), 아래에 레벨·가진 룬
// - 레벨업: 능력치 다섯 줄 [지금 → 목표] [−][+] 와 바뀔 값 미리 보기(예: 최대 HP 100 → 118),
//   필요한 룬 / 가진 룬 (모자라면 빨강), "확정" 버튼. 확정이 거절되면 비용 줄이 좌우로 흔들려요.
// - 창은 열릴 때 0.2초 동안 톡 나타나요.

#include "engine/core/math.hpp"

#include <optional>

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class CushionMenu;
class HeroProgress;
}  // namespace wind::game

namespace wind::client {

class CushionMenuView {
public:
    // 화면 한 장마다: 열려 있으면 나타나기·흔들림 시계를 돌려요
    void update(float dt_seconds, const game::CushionMenu& menu);
    void draw(engine::UiPainter& painter, engine::Vec2 screen, const game::CushionMenu& menu,
              const game::HeroProgress& progress) const;

private:
    void draw_main(engine::UiPainter& painter, engine::Vec2 screen, const game::CushionMenu& menu,
                   const game::HeroProgress& progress) const;
    void draw_level_up(engine::UiPainter& painter, engine::Vec2 screen, const game::CushionMenu& menu,
                       const game::HeroProgress& progress) const;

    float appear_ = 0.0f;                  // 열릴 때 0 -> 1
    int last_screen_ = -1;                 // 지난 화면 (화면이 바뀌면 다시 톡 나타나요)
    float shake_ = 0.0f;                   // 확정 거절 흔들림 남은 시간
    std::optional<unsigned> denied_seen_;  // 마지막으로 본 거절 횟수
};

}  // namespace wind::client
