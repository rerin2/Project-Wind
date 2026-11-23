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

#include "engine/fx/particle_drawing.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/render/render_queue.hpp"
#include "engine/resources/resource_manager.hpp"
#include "game/combat/sim_time_scale.hpp"
#include "game/entities/player_motion.hpp"
#include "game/input/input_bindings.hpp"
#include "game/input/movement_intent.hpp"
#include "game/presentation/player_sprite.hpp"
#include "game/presentation/ambient_particles.hpp"
#include "game/presentation/death_fx.hpp"
#include "game/presentation/feel_settings.hpp"
#include "game/presentation/field_atmosphere.hpp"
#include "game/presentation/field_camera.hpp"
#include "game/presentation/cushion_view.hpp"
#include "game/presentation/field_combat_fx.hpp"
#include "game/presentation/field_effects.hpp"
#include "game/states/field_assets.hpp"
#include "game/states/field_combat.hpp"
#include "game/states/field_rest.hpp"
#include "game/states/field_arms.hpp"
#include "game/states/field_items.hpp"
#include "game/states/field_spells.hpp"
#include "game/states/field_runes.hpp"
#include "game/world/exit_watcher.hpp"
#include "game/states/field_textures.hpp"
#include "game/presentation/field_lighting.hpp"
#include "game/presentation/dust_emitter.hpp"

#include "wind/content/map_loader.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wind::engine {
class Renderer;
}

namespace wind::game {

class HeroProgress;  // 주인공 진행 상태 (참조로만 써서 미리 알려 두기만 해요, 13a)

// 디버그 화면(F3, 개발자용 정보판)에 보여 줄 필드 상태
struct FieldDebugInfo {
    std::string map_id;                                // 지금 맵 이름(영문 ID)
    engine::Vec2 player_feet;                          // 발 위치 (픽셀)
    world::TileCoord player_tile;                      // 발이 있는 칸
    world::Direction facing = world::Direction::Down;  // 바라보는 방향
    std::string_view state;                            // idle / moving / running / attacking / dodging
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
    float mana = 0.0f;                                 // 지금 MP (9e)
    float mana_max = 0.0f;                             // 최대 MP
    int hp = 0;                                        // 지금 HP (11c)
    int hp_max = 0;                                    // 최대 HP
    int combo_hit = 0;                                 // 콤보 몇 타째 휘두르는 중 (0 = 공격 안 함)
    std::string combat;                                // 허수아비 수와 마지막으로 맞힌 피해 (9c)
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
                                                            HeroProgress& progress,
                                                            const ItemWorld& items, const GearWorld& gear,
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
    [[nodiscard]] std::optional<world::MapExit> take_exit_request() { return exits_.take_request(); }
    // 쿠션에 앉은 지 잠깐 지났으면 그 쿠션 id를 한 번 돌려줘요 (13b). 게임 루프가 페이드 후 같은 맵을 새로 읽어요.
    [[nodiscard]] std::optional<std::string> take_rest_request() { return rest_.take_travel_request(); }
    // 이 맵의 쿠션 이름 (처음 찾았을 때 배너용, 13b). 없는 id면 빈 글자
    [[nodiscard]] std::string cushion_name(std::string_view cushion_id) const { return rest_.name_of(cushion_id); }
    // 처음 찾은 쿠션을 피어나게 해요 (퐁, 꽃잎, 빛, 13b. 연출만)
    void bloom_cushion(std::string_view cushion_id) { cushion_view_.bloom(cushion_id); }
    // 쿠션 메뉴 "떠나기" (13c): 앉아 있으면 기지개 켜고 일어나요
    bool stand_up() { return player_.stand_up(); }
    // 레벨업 뒤 새 능력치를 바로 적용하고 빛기둥 연출을 틀어요 (13c)
    void refresh_hero_stats();
    // "[E] 쉬기" 안내를 보여 줄 때인가요? (쿠션 가까이 서 있고 쉴 수 있는 상태, 13b)
    [[nodiscard]] bool rest_prompt() const { return rest_.prompt_cushion(player_) != nullptr; }
    // 소란스러워서 못 쉰 횟수 (UI가 바뀐 걸 보고 "주변이 너무 소란스럽다"를 띄워요, 13b)
    [[nodiscard]] unsigned rest_denied_count() const { return rest_.denied_count(); }
    // 지난번 뒤로 주운 아이템·가득 차서 못 주운 아이템 (왼쪽 아래 알림 줄용, 14b). 가져가면 비워요.
    // 무기 장착 알림(15a)도 같은 줄에 띄워요.
    [[nodiscard]] std::vector<LootNotice> take_loot_notices() {
        auto notices = items_.take_notices();
        for (auto& notice : arms_.take_notices()) notices.push_back(std::move(notice));
        return notices;
    }
    // 고른 마술과 Q로 바꾼 횟수 (HUD 마술 칸, 15b)
    [[nodiscard]] const FieldSpells& spells() const { return spells_; }
    // 인벤토리 화면 요청 (14c): 이 칸을 발 앞에 내려놓기, 가방 정렬. 다음 박자에 규칙대로 처리해요
    void request_put_down(std::size_t slot) { items_.request_put_down(slot); }
    void request_sort() { items_.request_sort(); }
    void request_use(std::size_t slot) { items_.request_use(slot); }  // 물약 마시기 (14d)
    void request_equip(std::size_t slot) { arms_.request_equip(slot); }  // 무기 장착 (15a)
    // 쿠션에서 쉬는 중인가요? (인벤토리를 열 수 없어요, 14c)
    [[nodiscard]] bool hero_resting() const { return player_.rest().active(); }
    // 지난번 뒤로 룬 흔적을 되찾았으면 그 양을 한 번 돌려줘요 ("룬을 되찾았다" 배너용, 14e)
    [[nodiscard]] std::optional<std::int64_t> take_rune_recovered() { return runes_.take_recovered(); }

    // 맵의 보여 줄 이름 (지역 이름 배너용, 예: "시험 초원")
    [[nodiscard]] const std::string& display_name() const { return map_.map.display_name; }
    // 주인공 HP (HUD 바용, 12a)
    [[nodiscard]] const Health& health() const { return player_.health(); }
    // 주인공이 쓰러져 있나요? (12b: "쓰러졌다" 화면, 12c: 부활)
    [[nodiscard]] bool hero_down() const { return player_.dead(); }
    // 주인공 HP·MP·스태미나 (12c: 맵을 옮길 때 새 필드로 들고 가요)
    [[nodiscard]] HeroVitals hero_vitals() const { return player_.vitals(); }
    // 들고 온 HP·MP·스태미나로 맞춰요 (12c: 출구로 들어온 새 필드에서만. 부활은 가득 찬 채 시작해요)
    void restore_hero(const HeroVitals& vitals) { player_.restore(vitals); }
    // 화면을 흑백으로 만들 정도 0~1 (12b, 렌더러가 월드에만 입혀요)
    [[nodiscard]] float desaturation() const { return death_fx_.grayscale(); }
    // 주인공 스태미나 (HUD 바용)
    [[nodiscard]] const Stamina& stamina() const { return player_.stamina(); }
    // 주인공 MP (HUD 바용)
    [[nodiscard]] const Mana& mana() const { return player_.mana(); }
    // 흔들림·번쩍임 세기 (접근성, --shake / --flash). 맵을 옮길 때마다 새 필드에 다시 알려 줘요.
    void set_feel(const FeelSettings& settings) { combat_fx_.set_settings(settings); }
    // 떠 있는 피해 숫자와, 그것을 화면 위치로 바꿀 카메라 (app 층이 글자로 그려요)
    [[nodiscard]] const DamageNumbers& damage_numbers() const { return combat_fx_.damage_numbers(); }
    [[nodiscard]] const engine::Camera2D& camera() const { return camera_rig_.camera(); }
    // MP가 모자라 마술이 안 나간 횟수 (HUD가 바뀐 걸 보고 MP 바를 깜빡여요)
    [[nodiscard]] unsigned mana_denied_count() const { return player_.mana_denied_count(); }

    // 주인공 몸 가운데가 보이는 화면 위치 (아이리스 전환의 가운데)
    [[nodiscard]] engine::Vec2 focus_on_screen() const;

    // 디버그 화면용 정보
    [[nodiscard]] FieldDebugInfo debug_info() const;

private:
    // 싸움·쿠션 그림 (파일에서 읽어서 리소스 관리자가 정리해요)
    struct FieldSprites {
        engine::TextureHandle slash;                   // 칼자국 반달
        std::vector<engine::TextureHandle> dummies;    // 허수아비마다 그림 (assets.dummies와 같은 순서)
        std::vector<engine::TextureHandle> monsters;   // 몬스터마다 그림 (assets.monsters와 같은 순서, 11a)
        engine::TextureHandle cushion;                 // 캣닢 쿠션 그림 판 (13b)
        engine::TextureHandle item_icons;              // 아이템 아이콘 그림 판 (14b)
    };

    // create()를 통해서만 만들어요. (준비물 검사를 빼먹지 않도록)
    FieldState(engine::Renderer& renderer, FieldAssets assets, engine::TextureHandle tileset_texture,
               engine::TextureHandle player_texture,
               FieldSprites sprites,
               std::string_view start_weather, engine::Vec2 viewport, std::uint32_t stress_lights,
               HeroProgress& progress, const ItemWorld& items, const GearWorld& gear, MapLoot& loot,
               std::string_view map_path);

    // 지금 화면에 그릴 발 위치 (두 박자 사이를 이은 위치)
    [[nodiscard]] engine::Vec2 drawn_feet() const;

    content::LoadedMap map_;                 // 맵과 타일셋 정보
    PlayerSprite sprite_;                    // 캐릭터 그림: 동작 넘기기, 구르기 잔상, 그리기
    engine::TextureHandle tileset_texture_;  // 맵 타일 그림
    FieldTextures textures_;                 // 코드로 만든 그림자·파티클·빛 그림 (필드가 끝나면 지워요)
    FieldEffects effects_;                   // 파티클 효과 요리법들 (아래 먼지·분위기가 빌려 써요)
    FieldCamera camera_rig_;                 // 사진사: 주인공을 따라가며 가는 쪽을 조금 더 보여 줘요
    PlayerMotion player_;                    // 캐릭터의 몸: 다리(이동) + 스태미나 + 구르기 버튼 메모장
    FieldCombat combat_;                     // 허수아비들과 칼 판정 (9c)
    HeroProgress& progress_;                 // 주인공 진행 상태 (능력치·룬, 앱이 가지고 필드는 빌려 써요, 13a)
    FieldRest rest_;                         // 캣닢 쿠션: E로 쉬기·일어나기, 쉬기 이동 요청 (13b)
    SimTimeScale time_scale_;                // 시뮬레이션 시간 배수: 맞는 순간 멈칫(10a) × 쓰러질 때 슬로모션(12b)
    DeathFx death_fx_;                       // 쓰러짐 연출 시계: 흑백 정도, 눕는 정도 (12b)
    FieldCombatFx combat_fx_;               // 싸움 연출: 허수아비·칼자국·빛 탄·번쩍임·흔들림·피해 숫자
    CushionView cushion_view_;              // 쿠션 연출: 시든·피어난 모습, 초록 빛, 하트·z (13b)
    FieldRunes runes_;                      // 룬: 받기·잃기, 룬 흔적과 되찾기 (13a, 14e)
    FieldItems items_;                      // 아이템: 드롭·자동 줍기(규칙)와 튀기·반짝·빨려 들기(연출), 알림 (14b)
    FieldArms arms_;                        // 무기: 끼고 있는 무기 쥐여 주기, 장착 요청 (15a)
    FieldSpells spells_;                    // 마술: 고른 마술 쥐여 주기, Q (15b)
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
    ExitWatcher exits_;                      // 출구를 밟았나 지켜보기 (게임 루프가 요청을 가져가요)
    std::uint32_t tiles_queued_ = 0;         // 이번 화면에 넣은 타일 수
    float scene_seconds_ = 0.0f;             // 이 맵에 들어온 뒤 흐른 시간 (움직이는 타일용)
};

}  // namespace wind::game
