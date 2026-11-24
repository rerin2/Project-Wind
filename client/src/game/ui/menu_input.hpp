#pragma once

// 이 파일은 무엇을 하나요?
// 쿠션 메뉴와 그 안의 화면들(레벨업, 마술 칸)이 받는 입력이에요. 게임 루프가 키를 이것으로 바꿔 줘요.
// 15f에서 마술 칸 화면도 쓰게 되어 cushion_menu.hpp에서 따로 뺐어요.

#include <cstdint>

namespace wind::game {

enum class MenuInput : std::uint8_t { Up, Down, Left, Right, Confirm, Back };

}  // namespace wind::game
