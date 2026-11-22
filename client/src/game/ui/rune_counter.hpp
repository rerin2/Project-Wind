#pragma once

// 이 파일은 무엇을 하나요?
// HUD 오른쪽 아래 룬 숫자의 움직임이에요. (명세 13.2, 13c) 그리지 않고 숫자만 다뤄서 창 없이 테스트할 수 있어요.
// - 룬이 늘면: "+12"가 잠깐 떠오르고, 보이는 숫자가 0.6초 동안 굴러가듯 올라가요 (빠르게 시작해 천천히 멈춤)
// - 룬이 줄면(쓰러짐·레벨업): 숫자가 빨갛게 0.8초 동안 내려가고 "-6"이 떠요
// - 처음 본 값은 굴리지 않고 바로 보여 줘요 (게임을 켜자마자 0에서 굴러가지 않게)

#include <cstdint>

namespace wind::game {

class RuneCounter {
public:
    // 화면 한 장마다: actual = 진짜 가진 룬
    void update(float dt_seconds, std::int64_t actual);

    // 지금 보여 줄 숫자 (굴러가는 중이면 중간값)
    [[nodiscard]] std::int64_t shown() const;
    // 줄어드는 중인가요? (빨간 글자)
    [[nodiscard]] bool losing() const { return rolling() && to_ < from_; }
    // 떠 있는 "+N"/"-N" (0이면 없음), 투명도 0~1, 떠오른 높이 (픽셀)
    [[nodiscard]] std::int64_t popup() const { return popup_seconds_ > 0.0f ? popup_amount_ : 0; }
    [[nodiscard]] float popup_alpha() const;
    [[nodiscard]] float popup_rise() const;

private:
    [[nodiscard]] bool rolling() const { return roll_seconds_ < roll_length_; }

    bool started_ = false;
    std::int64_t from_ = 0;        // 굴리기 시작한 숫자
    std::int64_t to_ = 0;          // 굴러갈 목표 (진짜 룬)
    float roll_seconds_ = 0.0f;    // 굴리기 시작한 뒤 시간
    float roll_length_ = 0.0f;     // 이번 굴리기 길이 (늘 때 0.6초, 줄 때 0.8초)
    std::int64_t popup_amount_ = 0;   // 떠 있는 +N/-N (계속 늘면 더해져요)
    float popup_seconds_ = 0.0f;      // 남은 시간
};

}  // namespace wind::game
