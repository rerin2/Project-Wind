#pragma once

// 이 파일은 무엇을 하나요?
// 화면 오른쪽 위 "저장했어요" 아이콘의 움직임이에요. (명세 12.3 "저장 중임을 화면 구석 아이콘으로", 16c)
// 그리지 않고 시간·투명도만 다뤄서 창 없이 테스트할 수 있어요. 그리기는 app 층 SaveIconView.
// - 저장 횟수가 늘면(쉬기·쿠션 메뉴·쓰러짐·지역 전환 어느 저장이든) 0.2초에 나타나고,
//   마지막 저장 뒤 1.2초 동안 보였다가 0.4초에 사라져요. 보이는 동안 또 저장하면 다시 1.2초.
// - 처음 본 횟수는 저장으로 치지 않아요 (게임을 켜자마자 뜨지 않게).

namespace wind::game {

class SaveIcon {
public:
    static constexpr float kFadeInSeconds = 0.2f;   // 나타나기
    static constexpr float kHoldSeconds = 1.2f;     // 마지막 저장 뒤 보이는 시간 (나타나기 포함)
    static constexpr float kFadeOutSeconds = 0.4f;  // 사라지기

    // 화면 한 장마다: saves_done = 지금까지 성공한 저장 횟수 (SaveKeeper)
    void update(float dt_seconds, unsigned saves_done);

    // 보이나요? (그릴지 말지)
    [[nodiscard]] bool visible() const { return alpha() > 0.0f; }
    // 진하기 0~1
    [[nodiscard]] float alpha() const;
    // 나타난 뒤 흐른 시간 (초, 반짝이 돌기·잎 흔들림용). 다시 저장해도 이어서 세요
    [[nodiscard]] float shown_seconds() const { return shown_; }

private:
    bool started_ = false;     // 저장 횟수를 한 번이라도 봤나요?
    unsigned last_count_ = 0;  // 지난번에 본 저장 횟수
    bool active_ = false;      // 나타나 있는 중 (사라지기가 끝나면 false)
    float since_save_ = 0.0f;  // 마지막 저장 뒤 흐른 시간
    float fade_in_ = 0.0f;     // 나타나기 진행 (초, 이미 보이면 다시 처음부터 하지 않아요)
    float shown_ = 0.0f;       // 나타난 뒤 흐른 시간
};

}  // namespace wind::game
