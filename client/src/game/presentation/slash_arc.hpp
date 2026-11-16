#pragma once

// 이 파일은 무엇을 하나요?
// 칼을 휘두를 때 보이는 반달 모양 칼자국이에요. (9단계 9c "휘두르기 연출")
// 주인공 공격 그림이 아직 없어서, 판정 구간이 시작되면 히트박스 자리에 하얀 반달을 띄워요.
// - 바라보는 방향으로 돌려서 그려요 (오른쪽 0도, 아래 90도, 왼쪽 180도, 위 270도).
// - 짧게 휙 쓸고(각도가 조금 움직임) 빛나며 사라져요. 2타는 반대 방향으로 쓸어요.
// 보여 주기 전용이라 판정·피해에는 영향이 없어요.

#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"
#include "game/combat/attack_runner.hpp"

#include "wind/world/direction.hpp"

#include <optional>

namespace wind::game {

class SlashArc {
public:
    explicit SlashArc(engine::TextureHandle texture);

    // 화면 한 장마다: 판정 구간이 새로 시작됐으면 칼자국을 만들고, 있던 칼자국은 나이를 먹어요.
    void update(float dt_seconds, const AttackRunner& attack, const std::optional<engine::RectF>& hitbox,
                world::Direction facing);
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const;

    [[nodiscard]] bool visible() const { return age_ < lifetime_; }

private:
    engine::TextureHandle texture_;  // 흰 반달 그림 (fx/slash_arc.bmp)
    engine::RectF box_;              // 칼자국을 그릴 히트박스 (맵 위)
    world::Direction facing_ = world::Direction::Right;  // 휘두른 방향
    bool reverse_ = false;           // 반대 방향으로 쓸기 (짝수 번째 콤보)
    float age_ = 1.0f;               // 생긴 뒤 흐른 시간
    float lifetime_ = 0.0f;          // 보이는 시간 (0이면 안 보임)
    bool was_active_ = false;        // 지난 화면에 판정 구간이었나 (새로 시작했는지 알려고)
};

}  // namespace wind::game
