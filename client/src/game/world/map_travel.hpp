#pragma once

// 이 파일은 무엇을 하나요?
// 맵 옮기기 담당이에요. (명세 7.4, 12c 부활)
// 1) 주인공이 출구를 밟으면 begin(출구): 화면을 가리기 시작해요 (페이드 또는 아이리스).
//    부활(12c)도 같은 길로 가요: "지금 맵의 기본 자리"를 출구처럼 넘기고, respawn 표시와 느린 페이드를 붙여요.
// 2) 화면이 다 가려지면 enter(출구, 부활인가)를 불러 새 맵을 열어 달라고 해요.
// 3) 새 맵이 열렸든 못 열렸든 화면을 다시 보여 줘요.
//    못 열었으면 지금 맵에 그대로 있고, 몇 초 동안 화면에 보여 줄 오류 글자를 남겨요.
// SDL도 그림도 몰라서, 창 없이 "잘못된 출구" 경우를 테스트할 수 있어요.

#include "engine/fx/screen_fade.hpp"

#include "wind/world/map.hpp"

#include <functional>
#include <optional>
#include <string>

namespace wind::game {

// 이번 이동의 설정 (begin에 넘겨요)
struct TravelOptions {
    bool respawn = false;       // 쓰러진 주인공의 부활인가 (12c: 새 맵에서 HP·MP·스태미나 가득)
    float fade_seconds = 0.0f;  // 이번만 다른 페이드 길이 (0이면 기본)
};

class MapTravel {
public:
    // 새 맵을 열어 주는 일. 열었으면 true, 못 열었으면(파일 없음, 등장 자리 없음 등) false.
    // respawn: 부활로 여는 맵인가 (아니면 보통 출구라 주인공 HP 등을 들고 가요)
    using EnterMap = std::function<bool(const world::MapExit&, bool respawn)>;

    // fade_seconds: 가리기(또는 다시 보여 주기)에 걸리는 기본 시간
    explicit MapTravel(float fade_seconds);

    // 출구로 떠나기 시작해요. 이미 옮기는 중이면 무시하고 false.
    bool begin(world::MapExit exit, TravelOptions options = {});
    // 화면 한 장마다: 가리기·보여 주기를 진행하고, 다 가려진 순간 enter를 한 번 불러요.
    void update(float dt_seconds, const EnterMap& enter);

    // 옮기는 중인가요? (이때는 게임 입력을 막아요)
    [[nodiscard]] bool busy() const { return fade_.busy(); }
    [[nodiscard]] const engine::ScreenFade& fade() const { return fade_; }
    // 화면에 잠깐 보여 줄 오류 글자 (보여 줄 게 없으면 빈 글자)
    [[nodiscard]] const std::string& notice() const { return notice_; }

private:
    engine::ScreenFade fade_;
    std::optional<world::MapExit> pending_;  // 화면이 다 가려지면 열 출구
    bool pending_respawn_ = false;           // 그 출구가 부활인가
    std::string notice_;                     // 마지막 실패 이유
    float notice_seconds_ = 0.0f;            // 오류 글자를 몇 초 더 보여 줄지
};

}  // namespace wind::game
