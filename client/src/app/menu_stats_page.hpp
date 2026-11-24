#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴 "능력치" 탭 그리기 (15e, 보기만, 명세 13.2: 레벨과 능력치는 HUD 대신 메뉴에서).
// - 위: 레벨과 가진 룬
// - 능력치 다섯 줄 (생명력·정신력·지구력·힘·지능)
// - 아래: 그 능력치로 정해진 값들 (최대 HP·MP·스태미나, 물리·마법 공격력, 들 수 있는 무게, 마술 칸)

namespace wind::client {

struct MenuPage;

void draw_stats_page(const MenuPage& page);

}  // namespace wind::client
