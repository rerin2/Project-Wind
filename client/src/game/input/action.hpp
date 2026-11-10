#pragma once

// 이 파일은 무엇을 하나요?
// 게임에서 할 수 있는 "동작" 목록이에요. 예: 위로 가기, 구르기, 끄기
// 게임 코드는 "W 키가 눌렸니?"가 아니라 "위로 가기가 눌렸니?"라고 물어봐요.
// 어떤 키가 어떤 동작인지는 InputBindings(키 연결표)가 정해요.

#include <cstddef>
#include <cstdint>

namespace wind::game {

enum class Action : std::uint8_t {
    MoveUp,           // 위로 가기
    MoveDown,         // 아래로 가기
    MoveLeft,         // 왼쪽으로 가기
    MoveRight,        // 오른쪽으로 가기
    Dodge,            // 구르기 (짧고 빠르게 피하기)
    ToggleDebug,      // 디버그 정보 표시 (개발자용 정보 화면 켜기/끄기)
    ToggleCollision,  // 충돌 표시 (못 지나가는 칸과 발 상자 보여 주기)
    Screenshot,       // 스크린샷 찍기 (지금 화면을 그림 파일로 저장)
    SkipTime,         // 시간 3시간 앞으로 (디버그)
    NextWeather,      // 다음 날씨로 (디버그)
    Quit,             // 게임 끄기
    Count  // 개수 표시용 (진짜 동작은 아니고, 동작이 몇 개인지 세는 데 써요)
};

inline constexpr std::size_t kActionCount = static_cast<std::size_t>(Action::Count);

}  // namespace wind::game
