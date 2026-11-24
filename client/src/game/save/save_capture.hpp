#pragma once

// 이 파일은 무엇을 하나요?
// 지금 게임 상태를 저장 모양(SaveData)으로 옮겨 적어요. (명세 12.1, 16b)
// 파일에 쓰는 일은 SaveSlots가, 언제 쓸지는 앱(SaveKeeper)이 정해요.
// 저장한 때·플레이 시간·게임 버전은 앱이 채워요(이 함수는 시계를 보지 않아 테스트가 쉬워요).

#include "game/save/save_sources.hpp"

#include "wind/save/save_data.hpp"

namespace wind::game {

[[nodiscard]] save::SaveData capture_save(const SaveSources& sources, const HeroPlace& hero);

}  // namespace wind::game
