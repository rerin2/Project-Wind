#pragma once

// 이 파일은 무엇을 하나요?
// 저장에 들어가는 게임 상태가 어디 있는지 한 곳에 모은 "주소록"이에요. (16b)
// 모두 필드 화면 밖(ClientApp)에 하나씩 있어서 맵을 옮겨도 남는 것들이에요.
// 주인공 위치·HP·MP는 지금 필드에 있어서 따로(HeroPlace) 받아요.
// 게임 규칙 쪽이라 SDL을 몰라요.

#include "engine/core/math.hpp"

#include <string>

namespace wind::items {
class Inventory;
class Equipment;
class QuickSlots;
}  // namespace wind::items

namespace wind::game {

class HeroProgress;
class LootStore;
class GameClock;

struct SaveSources {
    HeroProgress& progress;   // 능력치·룬·흔적·쿠션·마술 공책
    items::Inventory& bag;    // 가방
    items::Equipment& worn;   // 끼고 있는 장비
    items::QuickSlots& quick; // 단축 칸 1~4
    LootStore& loot;          // 맵마다 바닥 아이템
    GameClock& clock;         // 게임 시계
};

// 지금 필드의 주인공 (어느 맵 어디, HP·MP)
struct HeroPlace {
    std::string map;     // 맵 파일 (콘텐츠 폴더 기준)
    engine::Vec2 feet;   // 발 위치 (맵 위 픽셀)
    int hp = 0;
    float mp = 0.0f;
};

}  // namespace wind::game
