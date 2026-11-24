#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴 "장비" 탭 그리기 (15e, 보기만).
// - 위: 장비 무게 / 들 수 있는 무게 (무거우면 빨갛게 "무거움")
// - 칸 네 줄: 무기 · 머리 · 몸 · 장신구 (아이콘, 칸 이름, 아이템 이름 또는 "비어 있음"). 고른 줄은 금빛
// - 아래: 고른 칸의 설명. 무기면 공격력·힘 보정·치명타·한 번 휘두를 때 스태미나·콤보 수.
//   머리·몸·장신구는 확장 버전에서 생긴다는 안내.

#include <cstddef>

namespace wind::client {

struct MenuPage;

void draw_gear_page(const MenuPage& page, std::size_t focused_row);

}  // namespace wind::client
