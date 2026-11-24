#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴 "마술" 탭 그리기 (15e, 보기만).
// - 위: 마술 칸 (열린 칸 수 / 4, 칸마다 아이콘, 고른 칸은 금빛, 잠긴 칸은 흐리게)
// - 가운데: 배운 마술 목록 (아이콘, 이름, 칸에 들어 있으면 "칸 N"). 고른 줄은 금빛
// - 아래: 고른 마술 설명 (요구 지능 — 모자라면 빨강, MP, 시전 시간, 위력, 동작) + "칸 바꾸기는 캣닢 쿠션에서"

#include <cstddef>

namespace wind::client {

struct MenuPage;

void draw_spell_page(const MenuPage& page, std::size_t focused_row);

}  // namespace wind::client
