#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터를 "보여 주기만" 하는 담당이에요. (11a)
// - 발밑 그림자 (몸이 뜨면 조금 작아져요)
// - 몬스터 그림: 쉴 때 숨쉬기, 걸을 때 통통 뛰기 (monster_pose가 모양을 계산해요)
// - F4: 맞는 상자(노랑)와 벽에 부딪히는 발 상자(하늘색)
// 위치는 두 박자 사이를 이어서(보간) 그려서 부드러워요. 게임 규칙(위치·상태)은 바꾸지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"
#include "game/monsters/monster.hpp"

#include <span>
#include <vector>

namespace wind::game {

class MonsterView {
public:
    // shadow: 발밑 그림자 그림, textures: 몬스터마다 그림 (monsters와 같은 순서)
    MonsterView(engine::TextureHandle shadow, std::vector<engine::TextureHandle> textures);

    // 화면 한 장마다: 연출 시계와 마리마다 뜀 세기를 흘려요
    void update(float dt_seconds, std::span<const Monster> monsters);
    // 그림자·몬스터를 그리기 목록에 넣어요. show_boxes(F4)면 상자도.
    void queue(const engine::Camera2D& camera, std::span<const Monster> monsters, float alpha, bool show_boxes,
               engine::RenderQueue& queue) const;

private:
    void queue_monster(const engine::Camera2D& camera, const Monster& monster, std::size_t index, float alpha,
                       engine::RenderQueue& queue) const;
    // F4 상자는 판정과 같은 "지금 박자" 위치에 그려요 (보간하지 않아요)
    void queue_boxes(const engine::Camera2D& camera, const Monster& monster,
                     engine::RenderQueue& queue) const;

    engine::TextureHandle shadow_;                 // 발밑 그림자
    std::vector<engine::TextureHandle> textures_;  // 몬스터 그림 (순서대로)
    std::vector<float> hop_blend_;                 // 마리마다 뜀 세기 0~1
    float clock_ = 0.0f;                           // 숨쉬기 시계 (초)
};

}  // namespace wind::game
