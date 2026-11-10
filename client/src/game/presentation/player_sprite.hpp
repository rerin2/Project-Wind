#pragma once

// 이 파일은 무엇을 하나요?
// 주인공 그림 담당이에요. (field_state.cpp에서 나눠 왔어요)
// - 그림 넘기기: 서 있기 / 걷기 / 달리기 / 구르기에 맞는 동작 그림과 넘기는 빠르기
// - 구르기 잔상: 구르는 동안 찍은 사진을 뒤에 흐리게 남겨요
// - 그리기 목록에 넣기: 그림 속 발 위치가 맵 위의 진짜 발 위치에 딱 오도록
// 다리(Mover)의 상태를 "보기만" 해요. 그림 때문에 캐릭터 위치가 바뀌는 일은 없어요.

#include "engine/animation/animator.hpp"
#include "engine/animation/sprite_sheet.hpp"
#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"
#include "game/entities/mover.hpp"
#include "game/presentation/afterimage_trail.hpp"

namespace wind::game {

class PlayerSprite {
public:
    // sheet: 그림 설명서 ("idle", "walk" 동작이 꼭 있어야 해요. field_assets가 미리 확인해요)
    PlayerSprite(engine::SpriteSheet sheet, engine::TextureHandle texture);

    // 화면 한 장마다: 동작 그림을 넘기고, 구르는 중이면 잔상 사진을 찍어요.
    // feet: 화면에 그릴 발 위치 (두 박자 사이를 이은 위치)
    void update(double dt_seconds, const Mover& mover, engine::Vec2 feet);
    // 잔상과 주인공을 그리기 목록에 넣어요. 둘 다 발 위치(y)로 줄을 서요.
    void queue(const engine::Camera2D& camera, engine::Vec2 feet, world::Direction facing,
               engine::RenderQueue& queue) const;

private:
    engine::SpriteSheet sheet_;
    engine::TextureHandle texture_;
    engine::Animator animator_;
    AfterimageTrail trail_;
};

}  // namespace wind::game
