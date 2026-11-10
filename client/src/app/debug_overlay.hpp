#pragma once

// 이 파일은 무엇을 하나요?
// 개발자용 정보판(디버그 오버레이)이에요. 화면 왼쪽 위에 글자로 보여 줘요. (명세 5.10)
// 자동차 계기판처럼 "지금 게임이 잘 돌아가나?"를 숫자로 확인할 수 있어요.
// - FPS(1초에 화면을 몇 장 그리나), 한 장 그리는 데 걸린 시간
// - 지금 맵, 캐릭터 칸 위치, 바라보는 방향
// - 그리기 횟수, 그림 파일 통계
// F3 키로 켜고 꺼요.

#include "engine/render/renderer.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/states/field_state.hpp"

#include <cstdint>

namespace wind::client {

class DebugOverlay {
public:
    // 보이기 <-> 숨기기 바꾸기
    void toggle() { visible_ = !visible_; }
    [[nodiscard]] bool visible() const { return visible_; }

    // 화면 한 장마다 걸린 시간을 기록해서 FPS와 평균 시간을 계산해요.
    void record_frame(double frame_seconds);

    // 정보판을 그려요. 숨김 상태면 아무것도 안 그려요.
    void draw(engine::Renderer& renderer, const game::FieldDebugInfo& field,
              const engine::ResourceStats& resources, std::uint64_t dropped_frames,
              std::string_view clock_text, std::string_view ui_text) const;

private:
    bool visible_ = false;  // 처음엔 숨겨요 (단계 8부터 왼쪽 위에 HUD가 있어서). F3으로 켜요
    // FPS는 0.5초 동안 모은 평균으로 계산해요. 매 장마다 바꾸면 숫자가 너무 깜빡거려서 못 읽어요.
    double window_seconds_ = 0.0;   // 이번 0.5초 동안 모은 시간
    std::uint32_t window_frames_ = 0;  // 이번 0.5초 동안 그린 장수
    double fps_ = 0.0;              // 마지막으로 계산한 FPS
    double frame_ms_ = 0.0;         // 마지막으로 계산한 한 장당 평균 시간(밀리초 = 1/1000초)
};

}  // namespace wind::client
