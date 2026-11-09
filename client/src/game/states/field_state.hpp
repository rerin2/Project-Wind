#pragma once

// 이 파일은 무엇을 하나요?
// "필드 화면"의 설계도예요. 필드 화면 = 맵 하나와 내 캐릭터가 나오고, 키보드로 돌아다니는 화면.
// 놀이판(맵) + 캐릭터 + 사진사(카메라) + 효과(먼지, 잔상, 그림자)를 한데 모아 둔 상자예요.
//
// 한 화면(프레임)에서 부르는 순서
// 1) handle_input  : 누른 키를 기억해요 (구르기는 메모장에 적어 둬요)
// 2) fixed_update  : 정해진 박자마다 게임 규칙 계산 (이동, 구르기, 벽 충돌)  ← 시뮬레이션
// 3) frame_update  : 그림 넘기기, 먼지, 잔상, 카메라                         ← 보여 주기(연출)
// 4) render        : 그릴 것들을 그리기 목록에 넣기
// 연출은 규칙 계산 결과를 "보기만" 해요. 연출 때문에 캐릭터 위치가 바뀌는 일은 없어요.

#include "engine/animation/animator.hpp"
#include "engine/animation/sprite_sheet.hpp"
#include "engine/camera/camera2d.hpp"
#include "engine/camera/look_ahead.hpp"
#include "engine/fx/particle_drawing.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/render/render_queue.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/entities/mover.hpp"
#include "game/input/input_bindings.hpp"
#include "game/input/input_buffer.hpp"
#include "game/input/movement_intent.hpp"
#include "game/presentation/afterimage_trail.hpp"
#include "game/presentation/ambient_particles.hpp"
#include "game/presentation/dust_emitter.hpp"

#include "wind/content/map_loader.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::engine {
class Renderer;
}

namespace wind::game {

// 디버그 화면(F3, 개발자용 정보판)에 보여 줄 필드 상태
struct FieldDebugInfo {
    std::string map_id;                                // 지금 맵 이름(영문 ID)
    engine::Vec2 player_feet;                          // 발 위치 (픽셀)
    world::TileCoord player_tile;                      // 발이 있는 칸
    world::Direction facing = world::Direction::Down;  // 바라보는 방향
    std::string_view state;                            // idle / walking / dodging
    float speed = 0.0f;                                // 지금 속도 (픽셀/초)
    std::uint32_t tiles_queued = 0;                    // 이번에 그리려고 넣은 타일 수
    std::size_t particles = 0;                         // 지금 떠 있는 파티클 수 (땅 + 공중)
    std::size_t ambient_effects = 0;                   // 맵 분위기 효과 개수
    std::uint64_t particles_dropped = 0;               // 통이 꽉 차서 버린 파티클 수
};

class FieldState {
public:
    // 맵·타일셋·캐릭터 그림을 불러와 필드 화면을 만들어요.
    // 맵이나 캐릭터 설명서(데이터)가 잘못됐으면 오류를 기록하고 nullptr(없음)를 돌려줘요.
    // 그림 파일만 없는 경우에는 체크무늬 대신 그림으로 계속 진행해요.
    [[nodiscard]] static std::unique_ptr<FieldState> create(engine::ResourceManager& resources,
                                                            engine::Renderer& renderer,
                                                            std::string_view map_path,
                                                            std::string_view player_sheet_path,
                                                            engine::Vec2 viewport,
                                                            std::uint32_t particle_stress = 0);

    // 1) 화면 한 장마다, 규칙 계산보다 먼저: 누른 키 기억
    void handle_input(const engine::InputState& input, const InputBindings& bindings);
    // 2) 정해진 박자마다(고정 스텝): 이동·구르기·벽 충돌 계산
    void fixed_update(double dt_seconds, const engine::InputState& input,
                      const InputBindings& bindings);
    // 3) 화면 한 장마다, 규칙 계산 다음: 연출과 카메라
    //    alpha: 지난 박자와 다음 박자 사이 어디쯤인지 (0~1), 위치를 부드럽게 이을 때 써요
    void frame_update(double dt_seconds, double alpha);
    // 4) 이번 화면에 그릴 것들을 그리기 목록(렌더 큐)에 넣어요.
    void render(engine::RenderQueue& queue);

    // 디버그 화면용 정보
    [[nodiscard]] FieldDebugInfo debug_info() const;

private:
    // 그림자와 파티클처럼 파일 없이 코드로 만드는 그림들
    struct EffectTextures {
        engine::TextureHandle shadow;
        engine::ParticleTextures particles;
    };

    // 파일에서 읽은 파티클 효과들. 못 읽은 효과는 비어 있어요(그 효과만 안 보여요).
    struct FieldEffects {
        std::optional<engine::ParticleEffect> dust_walk;
        std::optional<engine::ParticleEffect> dust_dodge;
        std::vector<engine::ParticleEffect> ambient;  // 맵 분위기 효과 (+ 시험용 부하 효과)
    };

    // create()를 통해서만 만들어요. (준비물 검사를 빼먹지 않도록)
    FieldState(content::LoadedMap map, engine::SpriteSheet sheet, engine::TextureHandle tileset_texture,
               engine::TextureHandle player_texture, EffectTextures textures, FieldEffects effects,
               engine::Vec2 viewport);

    // 지금 화면에 그릴 발 위치 (두 박자 사이를 이은 위치)
    [[nodiscard]] engine::Vec2 drawn_feet() const;
    void update_animation(double dt_seconds);
    void queue_player(engine::RenderQueue& queue, engine::Vec2 feet) const;
    void queue_body_box(engine::RenderQueue& queue, engine::Vec2 feet) const;

    content::LoadedMap map_;                 // 맵과 타일셋 정보
    engine::SpriteSheet sheet_;              // 캐릭터 그림 설명서
    engine::TextureHandle tileset_texture_;  // 맵 타일 그림
    engine::TextureHandle player_texture_;   // 캐릭터 그림
    EffectTextures textures_;                // 그림자·파티클 그림
    FieldEffects effects_;                   // 파티클 효과 요리법들 (아래 먼지·분위기가 빌려 써요)
    engine::Camera2D camera_;                // 사진사: 맵의 어느 부분을 화면에 보여 줄지
    engine::LookAhead look_ahead_;           // 가는 쪽을 조금 더 보여 주기
    Mover player_;                           // 캐릭터의 다리 (이동 규칙)
    MovementIntent intent_;                  // 어느 방향으로 가고 싶은지
    InputBuffer buffer_;                     // 구르기 버튼 메모장
    engine::Animator animator_;              // 캐릭터 그림 넘기기
    engine::ParticleRandom fx_random_;       // 파티클 주사위
    engine::ParticlePool ground_fx_;         // 땅 효과 통 (먼지): 캐릭터와 발 위치로 줄 서요
    engine::ParticlePool air_fx_;            // 공중 효과 통 (꽃잎, 반딧불): 나뭇잎보다 위
    DustEmitter dust_emitter_;               // 먼지 만드는 담당
    AmbientParticles ambient_;               // 맵 분위기 효과 담당
    AfterimageTrail trail_;                  // 구르기 잔상
    std::optional<engine::Vec2> dodge_burst_;  // 이번 박자에 구르기를 시작했으면 그 방향
    double alpha_ = 1.0;                     // 지금 화면이 두 박자 사이 어디쯤인지
    bool show_collision_ = false;            // F4 충돌 표시가 켜져 있나
    std::uint32_t tiles_queued_ = 0;         // 이번 화면에 넣은 타일 수
};

}  // namespace wind::game
