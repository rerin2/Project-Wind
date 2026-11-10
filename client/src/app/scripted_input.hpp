#pragma once

// 이 파일은 무엇을 하나요?
// 시험용 "가짜 키 누르기" 도우미 함수들이에요.
// 실행 옵션 --auto-move(한 방향키를 계속 누름)와 --press(정해진 장에서 키 한 번 누름)를
// 진짜 키보드 대신 키보드 상태(InputState)에 적어 넣어요.
// 운영체제에 키를 보내지 않아서, 다른 창(브라우저 등)에 키가 새어 나가지 않아요.

#include "app/client_options.hpp"
#include "engine/input/input_state.hpp"

#include "wind/world/direction.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace wind::client {

// 방향을 그 방향의 화살표 키로 바꿔요 (--auto-move에서 써요).
[[nodiscard]] engine::Key move_key(world::Direction dir);

// --press 키 이름을 엔진 키로 바꿔요. 모르는 이름이면 Unknown.
[[nodiscard]] engine::Key scripted_key(std::string_view name);

// frame번째 장: 그 장에 누르기로 한 키는 누르고, 바로 전 장에 눌렀던 키는 떼요.
void apply_scripted_presses(const std::vector<ClientOptions::KeyPress>& presses, std::uint64_t frame,
                            engine::InputState& input);

}  // namespace wind::client
