#pragma once

// 이 파일은 무엇을 하나요?
// 아이템 연출(바닥 아이템 LootView, 물약 마시기 DrinkView)이 함께 쓰는 그림·효과 묶음이에요. (14b, 14d)
// 필드가 만들 때 한 번 채워서 넘겨요. 그림·효과는 필드가 살아 있는 동안 그대로라 빌려 써도 안전해요.

#include "engine/fx/particle_effect.hpp"
#include "engine/render/render_types.hpp"

namespace wind::game {

// 아이템 연출에 쓰는 그림들
struct LootViewSetup {
    engine::TextureHandle icons;    // 아이콘 그림 판 (16×16 칸이 가로로 늘어섬, items/item_icons.bmp)
    engine::TextureHandle shadow;   // 발밑 그림자 그림
    engine::TextureHandle glow;     // 둥근 빛 그림 (반짝임)
    const engine::ParticleEffect* pickup_fx = nullptr;  // 주울 때 반짝 가루 (없으면 안 튀어요)
    const engine::ParticleEffect* heal_fx = nullptr;    // 물약 꿀꺽: HP 초록 가루 (DrinkView, 14d)
    const engine::ParticleEffect* mana_fx = nullptr;    // 물약 꿀꺽: MP 파란 가루 (DrinkView, 14d)
};

}  // namespace wind::game
