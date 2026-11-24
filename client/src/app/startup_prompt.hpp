#pragma once

// 이 파일은 무엇을 하나요?
// 게임 루프가 시작되기 전에 저장 문제를 묻는 작은 창이에요. (16b)
// 어두운 바탕 가운데 창 하나: 제목, 설명, 키 안내. 답(Enter/Esc)을 받을 때까지 화면을 계속 그려요.
// 창을 닫거나 --max-frames에 닿으면 "게임 끄기"로 답해요.
// 시험용 --press(enter·esc)와 --screenshot-at(--screenshot-dir를 줄 때만)도 받아요 (화면 확인 스크립트).

#include "game/ui/startup_question.hpp"

#include <cstdint>

namespace wind::engine {
class SdlPlatform;
class UiPainter;
}  // namespace wind::engine

namespace wind::game {
class InputBindings;
}

namespace wind::client {

struct ClientOptions;

// question을 보여 주고 답을 받아요. frame: 그린 장수 (게임과 같은 번호를 이어 써요)
[[nodiscard]] game::StartupQuestion::Answer ask_startup_question(const game::StartupQuestion& question,
                                                                 engine::SdlPlatform& platform,
                                                                 engine::UiPainter& painter,
                                                                 const game::InputBindings& bindings,
                                                                 const ClientOptions& options, std::uint64_t& frame);

}  // namespace wind::client
