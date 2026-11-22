#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 캣닢 쿠션에서 쉬는 순서(시간표)예요. (명세 8.7, 13b)
//   None     : 쉬지 않아요
//   Settling : 쿠션 가운데로 사뿐히 옮겨 가요 (짧게, 발이 쿠션 자리로 미끄러져요)
//   Sitting  : 쿠션 위에 앉아 쉬어요. 게임 입력은 막히고, E나 방향키를 누르면 일어나요
//              (13b 임시: 13c부터는 쿠션 메뉴의 "떠나기")
//   Standing : 기지개를 켜며 일어나요. 끝나면 None
// 앉아 있는 동안 몸이 어디로 얼마나 옮겨 갈지만 알려 주고, 실제로 옮기는 일은 PlayerMotion이 해요.
// 그림(앉기·웅크리기·기지개)은 이 단계와 시간을 보고 연출이 골라요. SDL을 몰라요.

#include "engine/core/math.hpp"

#include <cstdint>

namespace wind::game {

// 쉬기 시간 숫자 (이유는 docs/GAMEPLAY.md "Cat-nip cushions" 절)
struct RestTuning {
    float settle_seconds = 0.25f;  // 쿠션 가운데로 옮겨 가는 시간
    float stand_seconds = 0.6f;    // 기지개 켜고 일어나는 시간
};

class HeroRest {
public:
    enum class Phase : std::uint8_t { None, Settling, Sitting, Standing };

    explicit HeroRest(RestTuning tuning = {});

    // 쉬기 시작: 지금 발에서 seat(쿠션 자리)로 옮겨 가요
    void begin(engine::Vec2 seat);
    // 이미 쿠션 위에 있어요 (쉬기·부활로 새 맵을 연 직후): 바로 앉은 채 시작해요
    void sit_now(engine::Vec2 seat);
    // 일어나기 요청. 앉아 있을 때만 받아들여요 (옮겨 가는 중·일어나는 중이면 false)
    bool stand_up();
    // 한 박자. 이번 박자에 발이 옮겨 갈 거리(픽셀)를 돌려줘요 (Settling일 때만 0이 아니에요)
    engine::Vec2 step(float dt_seconds, engine::Vec2 feet);

    [[nodiscard]] Phase phase() const { return phase_; }
    [[nodiscard]] bool active() const { return phase_ != Phase::None; }
    // 지금 단계에 들어온 뒤 지난 시간 (그림 고르기용)
    [[nodiscard]] float phase_seconds() const { return phase_seconds_; }
    [[nodiscard]] const RestTuning& tuning() const { return tuning_; }

private:
    void enter(Phase phase);

    RestTuning tuning_;
    Phase phase_ = Phase::None;
    float phase_seconds_ = 0.0f;
    engine::Vec2 seat_{};  // 앉을 자리 (발 위치)
};

}  // namespace wind::game
