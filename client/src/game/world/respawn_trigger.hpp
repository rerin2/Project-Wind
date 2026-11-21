#pragma once

// 이 파일은 무엇을 하나요?
// 쓰러진 주인공을 "언제" 부활시킬지 정해요. (명세 9.5, 13.1, 12c, 개발자와 정함)
// - "쓰러졌다" 화면이 다 나타난 뒤(ready)부터 시간을 세요.
// - Enter(확인)를 누르면 바로, 아니면 2.5초 뒤 저절로 부활을 한 번 알려요(true).
// - 부활 맵을 못 열어서 아직 쓰러져 있으면, 2.5초 뒤 다시 알려요 (멈춰 있지 않게).
// - 서 있으면 처음으로 돌아가요.
// 언제만 정하고, 실제 부활(맵 다시 열기, 페이드)은 게임 루프가 해요. SDL을 몰라서 창 없이 테스트할 수 있어요.

namespace wind::game {

class RespawnTrigger {
public:
    // 화면 한 장마다. hero_down: 쓰러져 있나, screen_ready: 쓰러짐 화면이 다 나타났나(맵 옮기는 중이면 false로)
    // confirm: 이번 화면에 Enter를 눌렀나. 부활할 때면 true (한 번)
    bool update(float dt_seconds, bool hero_down, bool screen_ready, bool confirm);

    // 저절로 부활하기까지 남은 시간 (초, 기다리는 중이 아니면 kAutoSeconds)
    [[nodiscard]] float auto_left() const;

    static constexpr float kAutoSeconds = 2.5f;  // 쓰러짐 화면이 다 나타난 뒤 저절로 부활하기까지

private:
    float waited_ = 0.0f;  // 쓰러짐 화면이 다 나타난 뒤 기다린 시간
};

}  // namespace wind::game
