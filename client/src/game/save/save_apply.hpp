#pragma once

// 이 파일은 무엇을 하나요?
// 저장 모양(SaveData)을 게임 상태로 되살려요. (명세 12.1, 16b)
// 전부 성공 또는 전부 실패: 새 상태를 옆에서 다 만들어 본 뒤 한 번에 바꿔요.
// 콘텐츠와 맞는지(아이템·마술·맵이 있는지)는 먼저 check_save_content로 확인하고 불러야 해요.
// 바닥 아이템을 다시 만들므로 필드 화면이 없을 때(게임 시작 전)만 불러요.
// 주인공 위치·HP·MP는 필드를 만들 때 앱이 넘겨요(여기서는 다루지 않아요).

#include "game/save/save_sources.hpp"

#include "wind/save/save_data.hpp"

namespace wind::items {
class ItemCatalog;
}

namespace wind::game {

// 되살렸으면 true. 틀린 곳이 있으면 이유를 로그에 남기고 아무것도 안 바꿔요.
bool apply_save(const save::SaveData& data, SaveSources& sources, const items::ItemCatalog& catalog);

}  // namespace wind::game
