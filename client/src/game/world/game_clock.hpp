#pragma once

// 이 파일은 무엇을 하나요?
// 게임 시계예요. (명세 6.4) 기본으로 실제 1분이 게임 1시간이라, 게임 하루는 실제 24분이에요.
// 게임 규칙 쪽 시계라서 정해진 박자(고정 스텝)마다 앞으로 가요.
// 맵을 옮겨도 시간이 이어지도록 필드 화면이 아니라 게임 전체가 하나 가지고 있어요.
// (나중에 저장 파일에 이 시각을 적어요.)

#include <string>

namespace wind::game {

class GameClock {
public:
    // start_hour: 시작 시각 (0~24), seconds_per_hour: 실제 몇 초가 게임 1시간인지
    GameClock(float start_hour, float seconds_per_hour);

    // 실제 시간이 dt초 흘렀어요.
    void advance(double dt_seconds);
    // 시계를 몇 시간 앞으로 돌려요 (디버그 F5).
    void skip_hours(float hours);

    // 지금 시각 0 이상 24 미만 (예: 18.5 = 오후 6시 30분)
    [[nodiscard]] float hour() const { return hour_; }
    // 시작한 뒤 며칠째인지 (0부터)
    [[nodiscard]] int day() const { return day_; }
    // "18:30" 모양의 글자
    [[nodiscard]] std::string text() const;

private:
    float hour_;
    float seconds_per_hour_;
    int day_ = 0;
};

}  // namespace wind::game
