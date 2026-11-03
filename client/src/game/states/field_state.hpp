#pragma once

// 이 파일은 무엇을 하나요?
// "필드 화면"의 설계도예요. 필드 화면 = 맵 하나와 내 캐릭터가 나오고, 키보드로 걸어 다니는 화면.
// 놀이판(맵) + 말(캐릭터) + 사진사(카메라)를 한데 모아 둔 상자라고 생각하면 돼요.
// 지금(단계 1)은 서버 없이 내 컴퓨터 혼자 움직여요. 단계 2부터는 서버가 이동을 결정해요.

#include "engine/animation/animation.hpp"
#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/entities/grid_mover.hpp"
#include "game/input/input_bindings.hpp"

#include "wind/content/map_loader.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace wind::game {

// 디버그 화면(F3, 개발자용 정보판)에 보여 줄 필드 상태
struct FieldDebugInfo {
    std::string map_id;                              // 지금 맵 이름(영문 ID)
    world::TileCoord player_tile;                    // 캐릭터가 있는 칸
    world::Direction facing = world::Direction::Down;  // 바라보는 방향
    bool moving = false;                             // 걷는 중인가
    std::uint32_t tiles_queued = 0;                  // 이번에 그리려고 넣은 타일 수
};

class FieldState {
public:
    // 맵·타일셋·캐릭터 그림을 불러와 필드 화면을 만들어요.
    // 맵이나 캐릭터 설명서(데이터)가 잘못됐으면 오류를 기록하고 nullptr(없음)를 돌려줘요.
    // 그림 파일만 없는 경우에는 체크무늬 대신 그림으로 계속 진행해요.
    [[nodiscard]] static std::unique_ptr<FieldState> create(engine::ResourceManager& resources,
                                                            std::string_view map_path,
                                                            std::string_view player_sheet_path,
                                                            engine::Vec2 viewport);

    // 화면 한 장마다: 누른 키 기억, F4 켜고 끄기, 그림 넘기기(애니메이션), 카메라 따라가기
    void frame_update(double dt_seconds, const engine::InputState& input,
                      const InputBindings& bindings);
    // 정해진 박자마다(고정 스텝): 캐릭터 이동 계산
    void fixed_update(double dt_seconds, const engine::InputState& input,
                      const InputBindings& bindings);
    // 이번 화면에 그릴 것들을 그리기 목록(렌더 큐)에 넣어요.
    void render(engine::RenderQueue& queue);

    // 디버그 화면용 정보
    [[nodiscard]] FieldDebugInfo debug_info() const;

private:
    // create()를 통해서만 만들어요. (준비물 검사를 빼먹지 않도록)
    FieldState(content::LoadedMap map, engine::SpriteSheet sheet, engine::TextureHandle tileset_texture,
               engine::TextureHandle player_texture, engine::Vec2 viewport);

    // 캐릭터 발이 맵 위 어느 픽셀에 있는지 (걸어가는 중이면 중간 위치)
    [[nodiscard]] engine::Vec2 player_feet() const;

    content::LoadedMap map_;              // 맵과 타일셋 정보
    engine::SpriteSheet sheet_;           // 캐릭터 그림 설명서
    engine::TextureHandle tileset_texture_;  // 맵 타일 그림
    engine::TextureHandle player_texture_;   // 캐릭터 그림
    engine::Camera2D camera_;             // 사진사: 맵의 어느 부분을 화면에 보여 줄지
    GridMover player_;                    // 캐릭터의 칸 이동 담당
    engine::Animator animator_;           // 캐릭터 그림을 넘겨 주는 담당 (움직이는 그림)
    MovementIntent intent_;               // 어느 방향으로 가고 싶은지
    bool show_collision_ = false;         // F4 충돌 칸 표시가 켜져 있나
    std::uint32_t tiles_queued_ = 0;      // 이번 화면에 넣은 타일 수
};

}  // namespace wind::game
