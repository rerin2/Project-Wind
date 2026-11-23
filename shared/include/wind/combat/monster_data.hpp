#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터 설명(데이터)이에요. (명세 9.6, 14.2) 숫자는 content/monsters/*.json에서 읽어요.
// 11a: 대기·배회 칸. 11b: 알아채기 시간과 공격 목록 (감지·공격·추적 한계 거리를 쓰기 시작해요).
// 11d: 맞으면 움찔하는 시간과 사라질 때 효과(선택). 13a: 쓰러뜨리면 주는 룬. 14b: 드롭 표(선택).

#include "wind/combat/attack.hpp"
#include "wind/combat/hurtbox_shape.hpp"
#include "wind/combat/sprite_strip.hpp"
#include "wind/items/drop_entry.hpp"

#include <optional>
#include <string>
#include <vector>

namespace wind::combat {

// 배회: 제자리(처음 놓인 곳) 둘레를 천천히 돌아다니기
struct WanderData {
    float radius = 40.0f;     // 제자리에서 이만큼 안쪽의 점을 골라 걸어가요 (픽셀)
    float speed = 18.0f;      // 배회 걸음 빠르기 (픽셀/초)
    float pause_min = 1.0f;   // 한 번 걷고 쉬는 시간의 최소 (초)
    float pause_max = 2.5f;   // 한 번 걷고 쉬는 시간의 최대 (초)
};

struct MonsterData {
    std::string id;               // "slime_green"
    std::string name;             // "초록 슬라임"
    int max_hp = 30;              // 가득 찬 HP
    float attack_power = 4.0f;    // 공격력 (피해 계산 9.3, 11c)
    float defense = 0.0f;         // 방어력 (피해 계산 9.3, 11d)
    float move_speed = 40.0f;     // 쫓아가고 돌아갈 때 빠르기 (픽셀/초, 11b)
    float detect_range = 120.0f;  // 이 거리 안에 주인공이 오면 알아채요 (픽셀, 11b)
    float attack_range = 28.0f;   // 이 거리 안이면 공격 예고를 시작해요 (픽셀, 11b)
    float leash_range = 260.0f;   // 제자리에서·주인공에게서 이보다 멀어지면 포기하고 돌아가요 (픽셀, 11b)
    float notice_seconds = 0.4f;  // 알아챈 뒤 느낌표를 띄우고 멈춰 있는 시간 (초, 11b)
    WanderData wander;            // 배회
    float body_width = 12.0f;     // 발 상자(벽 충돌) 가로 (픽셀)
    float body_height = 6.0f;     // 발 상자 세로 (픽셀)
    HurtboxShape hurtbox;         // 맞는 상자 (11d)
    SpriteStrip sprite;           // 그림: 0 = 보통, 1 = 공격 예고(웅크림, 11b)
    std::vector<AttackData> attacks;  // 공격 목록 (1~6개, 11b는 첫 번째만 써요)
    float stagger_seconds = 0.25f;    // 칼·마술에 맞으면 움찔하는 시간 (초, 11d). 넉백도 이 시간 동안 줄어들어요
    std::optional<std::string> death_fx;  // 쓰러져 사라질 때 터지는 파티클 효과 (선택, 11d)
    int rune_reward = 0;                  // 쓰러뜨리면 주인공이 받는 룬 (13a)
    float keep_away = 0.0f;               // 주인공이 이보다 가까우면 물러나요 (픽셀, 선택, 0이면 안 물러남, 13c 개구리)
    std::vector<items::DropEntry> drops;  // 쓰러뜨리면 떨어뜨리는 아이템 표 (선택, 비면 없음, 14b)
};

}  // namespace wind::combat
