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

#include "engine/camera/camera2d.hpp"
#include "engine/camera/look_ahead.hpp"
#include "engine/fx/particle_drawing.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/render/render_queue.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/entities/player_motion.hpp"
#include "game/input/input_bindings.hpp"
#include "game/input/movement_intent.hpp"
#include "game/presentation/player_sprite.hpp"
#include "game/presentation/ambient_particles.hpp"
#include "game/presentation/field_atmosphere.hpp"
#include "game/presentation/field_effects.hpp"
#include "game/states/field_assets.hpp"
#include "game/presentation/field_lighting.hpp"
#include "game/presentation/dust_emitter.hpp"

#include "wind/content/map_loader.hpp"

#include <cstdint>
#include <map>
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
    bool lighting = false;                             // 조명을 쓰는 맵인가
    engine::LightStats lights;                         // 빛 수 (전체, 그린 것, 건너뛴 것)
    bool outdoors = false;                             // 시간·날씨를 쓰는 바깥 맵인가
    float night = 0.0f;                                // 밤 정도 0~1
    std::string weather;                               // 날씨 이름 ("none"이면 날씨 표 없음, 바뀌는 중이면 "a -> b")
    float weather_intensity = 0.0f;                    // 날씨 세기 0~1
    float stamina = 0.0f;                              // 지금 스태미나
    float stamina_max = 0.0f;                          // 최대 스태미나
    bool exhausted = false;                            // 지침 상태인가
};

// 성능 시험용 부하 (실행 옵션 --fx-stress, --light-stress). 평소에는 0이에요.
struct FieldStress {
    std::uint32_t particles = 0;  // 화면에 더 띄울 파티클 수
    std::uint32_t lights = 0;     // 시작 위치 주변에 더 놓을 빛 수
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
                                                            std::string_view spawn_id = "default",
                                                            FieldStress stress = {},
                                                            std::string_view start_weather = {});

    // 이 화면이 코드로 만든 그림(그림자·파티클·빛)을 정리해요. 맵을 옮길 때마다 새로 만드니까요.
    ~FieldState();
    FieldState(const FieldState&) = delete;
    FieldState& operator=(const FieldState&) = delete;

    // 1) 화면 한 장마다, 규칙 계산보다 먼저: 누른 키 기억
    void handle_input(const engine::InputState& input, const InputBindings& bindings);
    // 2) 정해진 박자마다(고정 스텝): 이동·구르기·벽 충돌 계산
    void fixed_update(double dt_seconds, const engine::InputState& input,
                      const InputBindings& bindings);
    // 3) 화면 한 장마다, 규칙 계산 다음: 연출과 카메라
    //    alpha: 지난 박자와 다음 박자 사이 어디쯤인지 (0~1), 위치를 부드럽게 이을 때 써요
    //    hour: 지금 게임 시각 (0~24). 바깥 맵의 밝기와 밤 효과가 이걸 따라가요.
    void frame_update(double dt_seconds, double alpha, float hour);
    // 다음 날씨로 바꾸기 시작해요 (디버그 F6). 날씨 표가 없는 맵에서는 아무 일도 없어요.
    void next_weather() { atmosphere_.next_weather(); }
    // 4) 이번 화면에 그릴 것들을 그리기 목록(렌더 큐)에 넣고, 조명 준비물도 만들어요.
    void render(engine::RenderQueue& queue);
    // render() 다음에: 이번 화면의 조명 준비물 (조명 없는 맵이면 enabled = false)
    [[nodiscard]] const engine::LightingFrame& lighting() const {
        return lighting_frame_ != nullptr ? *lighting_frame_ : no_lighting_;
    }

    // 주인공이 출구를 밟았으면 그 출구를 한 번 돌려주고 비워요. 없으면 nullopt.
    // 게임 루프가 이걸 보고 화면을 어둡게 한 뒤 다른 맵으로 옮겨 가요.
    [[nodiscard]] std::optional<world::MapExit> take_exit_request() {
        auto request = std::move(exit_request_);
        exit_request_.reset();
        return request;
    }

    // 맵의 보여 줄 이름 (지역 이름 배너용, 예: "시험 초원")
    [[nodiscard]] const std::string& display_name() const { return map_.map.display_name; }
    // 주인공 스태미나 (HUD 바용)
    [[nodiscard]] const Stamina& stamina() const { return player_.stamina(); }

    // 주인공 몸 가운데가 보이는 화면 위치 (아이리스 전환의 가운데)
    [[nodiscard]] engine::Vec2 focus_on_screen() const;

    // 디버그 화면용 정보
    [[nodiscard]] FieldDebugInfo debug_info() const;

private:
    // 그림자와 파티클처럼 파일 없이 코드로 만드는 그림들
    struct EffectTextures {
        engine::TextureHandle shadow;
        engine::ParticleTextures particles;
        engine::TextureHandle light;  // 둥근 빛 그림
    };

    // create()를 통해서만 만들어요. (준비물 검사를 빼먹지 않도록)
    FieldState(engine::Renderer& renderer, FieldAssets assets, engine::TextureHandle tileset_texture,
               engine::TextureHandle player_texture, EffectTextures textures,
               std::string_view start_weather, engine::Vec2 viewport, std::uint32_t stress_lights);

    // 지금 화면에 그릴 발 위치 (두 박자 사이를 이은 위치)
    [[nodiscard]] engine::Vec2 drawn_feet() const;
    void queue_body_box(engine::RenderQueue& queue, engine::Vec2 feet) const;

    engine::Renderer* renderer_;             // 그림을 만들고 정리할 화가 (빌려 씀)
    content::LoadedMap map_;                 // 맵과 타일셋 정보
    PlayerSprite sprite_;                    // 캐릭터 그림: 동작 넘기기, 구르기 잔상, 그리기
    engine::TextureHandle tileset_texture_;  // 맵 타일 그림
    EffectTextures textures_;                // 그림자·파티클 그림
    FieldEffects effects_;                   // 파티클 효과 요리법들 (아래 먼지·분위기가 빌려 써요)
    engine::Camera2D camera_;                // 사진사: 맵의 어느 부분을 화면에 보여 줄지
    engine::LookAhead look_ahead_;           // 가는 쪽을 조금 더 보여 주기
    PlayerMotion player_;                    // 캐릭터의 몸: 다리(이동) + 스태미나 + 구르기 버튼 메모장
    MovementIntent intent_;                  // 어느 방향으로 가고 싶은지
    engine::ParticleRandom fx_random_;       // 파티클 주사위
    engine::ParticlePool ground_fx_;         // 땅 효과 통 (먼지): 캐릭터와 발 위치로 줄 서요
    engine::ParticlePool air_fx_;            // 공중 효과 통 (꽃잎, 반딧불): 나뭇잎보다 위
    DustEmitter dust_emitter_;               // 먼지 만드는 담당
    AmbientParticles ambient_;               // 맵 분위기 효과 담당
    FieldLighting lighting_;                 // 조명 담당 (주변광, 횃불, 주인공 빛)
    FieldAtmosphere atmosphere_;             // 시간 색, 날씨, 안개, 밤 효과
    const engine::LightingFrame* lighting_frame_ = nullptr;  // render()가 만든 조명 준비물
    engine::LightingFrame no_lighting_;      // render() 전에 물어보면 돌려줄 "조명 없음"
    std::optional<engine::Vec2> dodge_burst_;  // 이번 박자에 구르기를 시작했으면 그 방향
    double alpha_ = 1.0;                     // 지금 화면이 두 박자 사이 어디쯤인지
    bool show_collision_ = false;            // F4 충돌 표시가 켜져 있나
    bool exit_armed_ = false;                // 출구가 아닌 칸을 밟은 뒤라 출구가 동작하나
    std::optional<world::MapExit> exit_request_;  // 밟은 출구 (게임 루프가 가져가요)
    std::uint32_t tiles_queued_ = 0;         // 이번 화면에 넣은 타일 수
    float scene_seconds_ = 0.0f;             // 이 맵에 들어온 뒤 흐른 시간 (움직이는 타일용)
};

}  // namespace wind::game
