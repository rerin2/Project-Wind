#pragma once

// 이 파일은 무엇을 하나요?
// 모양이 맞는 저장이 "지금 콘텐츠"와도 맞는지 확인해요. (명세 15장, 16b, 2026-11-24 개발자와 정함)
// 하나라도 틀리면 그 저장은 손상으로 다뤄요(확인 창: 백업 불러오기 / 새로 시작). 조용히 버리지 않아요.
// - 아이템(가방·장비·단축 칸·바닥): 도감에 있고, 개수 ≤ 한 칸 상한, 장비는 맞는 칸, 무기는 무기 선반에,
//   단축 칸은 쓸 수 있는 아이템
// - 마술: 배운 마술 파일이 읽히고(마술 선반에 올려 둬요), 칸에 든 마술은 요구 지능 이상
// - 맵(위치·흔적·쿠션·바닥 아이템): 맵 파일이 읽히고, 주인공 위치는 맵 안의 걸을 수 있는 칸,
//   마지막 쿠션은 그 맵에 있는 쿠션
// 찾은 쿠션 목록은 확인하지 않아요(모르는 id는 피어난 모습 표시에만 쓰여 해가 없어요).

#include "wind/save/save_data.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace wind::items {
class ItemCatalog;
}

namespace wind::game {

class SpellShelf;
class WeaponRack;

struct SaveContent {
    std::filesystem::path content_root;  // 맵·마술 파일을 읽을 곳
    const items::ItemCatalog& catalog;   // 아이템 도감
    const WeaponRack& weapons;           // 무기 아이템 → 무기 데이터
    SpellShelf& spells;                  // 배운 마술을 여기 올려 둬요 (불러온 뒤 그대로 써요)
};

// 맞으면 true. 틀린 곳은 problems에 한 줄씩 적어요(로그와 확인 창용).
bool check_save_content(const save::SaveData& data, const SaveContent& content, std::vector<std::string>& problems);

}  // namespace wind::game
