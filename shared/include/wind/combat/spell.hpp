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
    int intelligence_required = 1;   // 요구 지능 (낮으면 마술 칸에 못 넣어요, 15b)
    int icon = 0;                    // 마술 아이콘 그림 판(ui/spell_icons.bmp)의 몇 번째 칸 (선택, 15b)
    int mp_cost = 0;                 // 한 번 쓸 때 MP (빛 탄·근접 마술)
    float mp_per_second = 0.0f;      // 유지하는 동안 1초에 쓰는 MP (광선만)
    float cast_seconds = 0.0f;       // 시전 시간: 누르고 이만큼 지나야 나가요
    float cast_move_scale = 0.4f;    // 시전하는 동안 걷는 빠르기 배수 (0 = 멈춤, 선택, 기본 0.4, 15c)
    float power = 1.0f;              // 위력 배수
    float knockback = 0.0f;          // 밀어내는 힘 (픽셀/초, 선택, 10a)
    float hitstop = 0.0f;            // 맞는 순간 멈칫 (초, 선택)
    float shake = 0.0f;              // 화면 흔들림 세기 0~1 (선택, 연출)
    SpellMotion motion = SpellMotion::Projectile;
    std::optional<ProjectileData> projectile;  // motion이 Projectile일 때만
    std::optional<HitboxShape> melee;          // motion이 Melee일 때만
    std::optional<BeamData> beam;              // motion이 Beam일 때만
    std::optional<std::string> fx;             // 빛 탄 꼬리 같은 효과 파일 (선택)
    std::optional<std::string> sfx;            // 소리 파일 (선택)
    std::optional<SpellLight> light;           // 마술이 내는 빛 (선택)
};

// 시전을 시작하려면 MP가 적어도 이만큼 있어야 해요 (15d).
// 빛 탄·근접은 한 번 쓰는 MP, 광선은 한 번 맞히는 동안(tick_seconds)의 MP예요 (혜성 광선 12 × 0.1 = 1.2).
// 시전기와 HUD(MP 모자라면 아이콘 흐림)가 같은 값을 써요.
[[nodiscard]] inline float start_mp(const Spell& spell) {
    if (spell.motion == SpellMotion::Beam && spell.beam) return spell.mp_per_second * spell.beam->tick_seconds;
    return static_cast<float>(spell.mp_cost);
}

}  // namespace wind::combat
