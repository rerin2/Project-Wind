#pragma once

// 이 파일은 무엇을 하나요?
// 마술 하나(예: 별빛 조각)를 적은 데이터예요. (명세 8.6)
// 마술은 셋 중 한 가지 "동작"을 가져요.
//   projectile: 빛 탄이 날아가요 (별빛 조각)
//   melee:      앞을 벰 (달빛 칼날). 칼 공격과 같은 히트박스 모양
//   beam:       누르고 있는 동안 광선 (혜성 광선). MP를 1초마다 써요
// 숫자는 content/spells/*.json에서 읽어요. 피해 = 마법 공격력 × power (명세 8.2, 9.3).

#include "wind/combat/attack.hpp"
#include "wind/combat/projectile_data.hpp"
#include "wind/world/rgba.hpp"

#include <optional>
#include <string>

namespace wind::combat {

// 광선 모양 (혜성 광선)
struct BeamData {
    float length = 0.0f;        // 광선 길이 (픽셀)
    float width = 0.0f;         // 광선 굵기 (픽셀)
    float tick_seconds = 0.1f;  // 몇 초마다 한 번씩 맞히는지
};

// 마술이 내는 빛 (명세 6.3 빛 데이터). 보여 주기 전용이에요.
struct SpellLight {
    float radius = 32.0f;  // 빛이 닿는 거리 (픽셀)
    world::Rgba color{255, 255, 255, 255};
    float intensity = 1.0f;  // 밝기 0~1
};

// 마술이 어떤 동작인지 (셋 중 꼭 하나)
enum class SpellMotion : unsigned char { Projectile, Melee, Beam };

struct Spell {
    std::string id;                  // "star_shard"
    std::string name;                // 화면에 보일 이름 ("별빛 조각")
    int intelligence_required = 1;   // 요구 지능 (낮으면 마술 칸에 못 넣어요, 15단계)
    int mp_cost = 0;                 // 한 번 쓸 때 MP (빛 탄·근접 마술)
    float mp_per_second = 0.0f;      // 유지하는 동안 1초에 쓰는 MP (광선만)
    float cast_seconds = 0.0f;       // 시전 시간: 누르고 이만큼 지나야 나가요
    float power = 1.0f;              // 위력 배수
    SpellMotion motion = SpellMotion::Projectile;
    std::optional<ProjectileData> projectile;  // motion이 Projectile일 때만
    std::optional<HitboxShape> melee;          // motion이 Melee일 때만
    std::optional<BeamData> beam;              // motion이 Beam일 때만
    std::optional<std::string> fx;             // 빛 탄 꼬리 같은 효과 파일 (선택)
    std::optional<std::string> sfx;            // 소리 파일 (선택)
    std::optional<SpellLight> light;           // 마술이 내는 빛 (선택)
};

}  // namespace wind::combat
