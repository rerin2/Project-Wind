// 이 파일은 무엇을 하나요?
// "필드 화면"(맵 위를 돌아다니는 화면)이 실제로 어떻게 움직이는지 적은 곳이에요.
// 맵과 캐릭터 그림을 불러오고, 키를 보고 캐릭터를 옮기고, 먼지·잔상·그림자를 만들고,
// 카메라를 따라가게 하고, 화면에 그릴 것들을 "그리기 목록"(렌더 큐)에 넣어요.

#include "game/states/field_state.hpp"

#include "engine/fx/particle_drawing.hpp"
#include "engine/map/coordinates.hpp"
#include "engine/map/tilemap_renderer.hpp"
#include "engine/render/renderer.hpp"
#include "game/physics/tile_collision.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/progression/hero_tuning.hpp"
#include "game/items/loot_store.hpp"
#include "game/presentation/debug_boxes.hpp"
#include "game/presentation/hero_blink.hpp"
#include "game/presentation/shadow_drawing.hpp"

#include "wind/log.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <random>
#include <string>
#include <utility>

namespace wind::game {
namespace {

// 구르기 버튼을 몇 초 동안 기억해 줄지 (명세 5.8 입력 버퍼 기본값)
constexpr double kInputBufferSeconds = 0.15;
// 파티클 통 크기 (이보다 많으면 버리고 세요, 명세 6.6)
constexpr std::size_t kGroundFxCapacity = 1024;  // 땅 효과 (먼지)
constexpr std::size_t kAirFxCapacity = 4000;     // 공중 효과 (꽃잎, 반딧불, 시험용 부하)
// 파티클 주사위 씨앗. 연출용이라 아무 숫자나 괜찮지만, 고정하면 매번 같은 모양이 나와요.
constexpr std::uint32_t kFxSeed = 20261109;

// F4: 발 상자 표시 색 (반투명 하늘색)
constexpr engine::Color kBodyBoxColor{80, 200, 255, 140};
// 휘두를 때 보이는 칼자국 그림 (흰 반달, 생성기 스크립트가 그려요)
constexpr std::string_view kSlashTexture = "fx/slash_arc.bmp";

// 읽은 효과가 있으면 그 주소, 없으면 nullptr (효과를 빌려 쓰는 담당들에게 넘겨요)
const engine::ParticleEffect* effect_ptr(const std::optional<engine::ParticleEffect>& effect) {
    return effect ? &*effect : nullptr;
}

}  // namespace

// 필드 화면 만들기: 파일 준비물을 읽고, 그림을 만들어요. 꼭 필요한 게 잘못되면 nullptr(없음).
std::unique_ptr<FieldState> FieldState::create(engine::ResourceManager& resources,
                                               engine::Renderer& renderer,
                                               std::string_view map_path,
                                               std::string_view player_sheet_path,
                                               engine::Vec2 viewport, HeroProgress& progress,
                                               const ItemWorld& items, std::string_view spawn_id,
                                               FieldStress stress, std::string_view start_weather) {
    // 1) 파일 준비물: 맵, 주인공 그림 설명서, 등장 자리, 하루 색, 날씨, 효과들
    auto assets = load_field_assets(resources.content_root(), map_path, player_sheet_path, spawn_id,
                                    stress.particles);
    if (!assets) return nullptr;  // 이유는 이미 기록했어요
    if (!FieldItems::drops_known(assets->monsters, items.catalog)) return nullptr;  // 드롭 아이템은 도감에 (14b)
    // 2) 그림 파일(텍스처). 없어도 체크무늬로 계속해요 (무엇이 없는지는 리소스 관리자가 기록해요)
    const auto tileset_texture = resources.texture(assets->map.tileset.image);
    const auto player_texture = resources.texture(assets->sheet.image);
    // 3) 싸움 그림: 칼자국, 허수아비·몬스터마다 그림 (같은 파일은 리소스 관리자가 한 번만 읽어요)
    FieldSprites sprites{resources.texture(kSlashTexture), {}, {}, resources.texture(kCushionTexture),
                         resources.texture(kItemIconsTexture)};
    for (const auto& dummy : assets->dummies) sprites.dummies.push_back(resources.texture(dummy.data().sprite.image));
    for (const auto& monster : assets->monsters) sprites.monsters.push_back(resources.texture(monster.data().sprite.image));
    if (stress.lights > 0) log::info("Light stress test: {} extra lights", stress.lights);
    log_field_entry(*assets);  // 맵 크기·효과·빛·허수아비 수 기록
    return std::unique_ptr<FieldState>(new FieldState(renderer, std::move(*assets), tileset_texture, player_texture,
                                                      std::move(sprites), start_weather, viewport, stress.lights,
                                                      progress, items, items.loot.on(map_path)));
}

// 준비물을 받아서 필드 화면을 실제로 꾸려요.
FieldState::FieldState(engine::Renderer& renderer, FieldAssets assets,
                       engine::TextureHandle tileset_texture, engine::TextureHandle player_texture,
                       FieldSprites sprites,
                       std::string_view start_weather, engine::Vec2 viewport,
                       std::uint32_t stress_lights, HeroProgress& progress, const ItemWorld& items,
                       MapLoot& loot)
    : map_(std::move(assets.map)),
      sprite_(std::move(assets.sheet), player_texture),
      tileset_texture_(tileset_texture),
      textures_(renderer),  // 그림자·파티클·빛 그림을 코드로 만들어요 (실패해도 그 효과만 안 보여요)
      effects_(std::move(assets.effects)),
      // 카메라: 화면 크기, 맵 크기(픽셀), 처음 볼 곳(등장 자리)
      camera_rig_(viewport,
                  engine::Vec2{static_cast<float>(map_.map.width), static_cast<float>(map_.map.height)} *
                      static_cast<float>(map_.map.tile_size),
                  static_cast<float>(map_.map.tile_size),
                  engine::tile_feet(assets.spawn_tile, map_.map.tile_size)),
      // 캐릭터가 처음 설 곳(등장 자리)과 능력치에서 나온 최대 HP·MP·스태미나 (13a)
      player_(engine::tile_feet(assets.spawn_tile, map_.map.tile_size), world::Direction::Down, MovementTuning{},
              stamina_tuning(progress.derived()), kInputBufferSeconds, std::move(assets.weapon),
              mana_tuning(progress.derived()), health_tuning(progress.derived())),
      // 허수아비들, 싸움 주사위 씨앗(치명타·몬스터, 켤 때마다 달라요), 기본 치명타 확률, 몬스터들 (11a)
      combat_(std::move(assets.dummies), std::random_device{}(), kBaseCritChance, std::move(assets.monsters)),
      progress_(progress),
      rest_(std::move(assets.cushions)),  // 맵의 캣닢 쿠션들 (13b)
      // 싸움 연출: 그림자·칼자국·빛 그림, 허수아비 그림, 빛 탄·불꽃 효과, 마술 빛
      combat_fx_(CombatFxSetup{textures_.shadow(), sprites.slash, textures_.light(),
                               std::move(sprites.dummies), std::move(sprites.monsters),
                               effect_ptr(effects_.star_trail),
                               effect_ptr(effects_.star_burst), effect_ptr(effects_.hit_spark),
                               assets.spell ? assets.spell->light : std::nullopt, textures_.vignette(),
                               &effects_.monster_fx, effect_ptr(effects_.spit_splash)}),
      // 쿠션 연출: 찾은 쿠션은 피어난 모습, 처음 찾으면 꽃잎 (13b)
      cushion_view_(rest_.cushions(), [&progress](std::string_view id) { return progress.discovered(id); },
                    sprites.cushion, effect_ptr(effects_.catnip_bloom), effect_ptr(effects_.level_up)),
      // 아이템: 이 맵의 바닥 목록, 아이콘·그림자·빛 그림, 주울 때 반짝, 물약 꿀꺽 가루(14d), 드롭 주사위 씨앗 (14b)
      items_(items, loot, {sprites.item_icons, textures_.shadow(), textures_.light(), effect_ptr(effects_.item_pickup),
                          effect_ptr(effects_.potion_heal), effect_ptr(effects_.potion_mana)}, std::random_device{}()),
      fx_random_(kFxSeed),
      ground_fx_(kGroundFxCapacity),
      air_fx_(kAirFxCapacity),
      // effects_는 위에서 이미 만들어졌고, 이 화면이 사라질 때까지 그대로라서 빌려 써도 안전해요.
      dust_emitter_(effect_ptr(effects_.dust_walk), effect_ptr(effects_.dust_dodge)),
      // 조명: 맵 분위기 + (시험용이면) 시작 위치 주변에 빛을 더 놓아요.
      lighting_(map_.map.ambience, map_.map.tile_size, effects_.light_fx, stress_lights,
                player_.mover().feet()),
      // 시간·날씨: 날씨 주사위 씨앗은 켤 때마다 달라서, 매번 다른 날씨로 시작해요.
      atmosphere_(std::move(assets.day_cycle), std::move(assets.weather), effects_.weather_fx, effects_.night,
                  std::random_device{}(), start_weather) {
    for (const auto& effect : effects_.ambient) ambient_.add(effect);  // 꽃잎·반딧불 같은 분위기 효과
    player_.set_spell(std::move(assets.spell));  // 고른 마술 (9e: 별빛 조각)
    combat_.set_hero_stats(progress_.derived());   // 칼·빛 탄 공격력의 힘·지능 보너스 (13a)
    if (assets.start_cushion) rest_.sit_on(*assets.start_cushion, player_);  // 쉬기·부활: 쿠션에 앉은 채 (13b)
}

FieldState::~FieldState() = default;  // 코드로 만든 그림은 FieldTextures가 지워요

void FieldState::handle_input(const engine::InputState& input, const InputBindings& bindings) {
    intent_.update(bindings, input);  // 새로 누른 방향키를 기억
    // 구르기는 바로 하지 않고 메모장에 적어 둬요. 다음 박자 계산 때 꺼내 써요.
    // (이번 화면에 박자 계산이 0번이어도 버튼이 사라지지 않아요)
    if (bindings.pressed(Action::Dodge, input)) player_.press_dodge();
    if (bindings.pressed(Action::Attack, input)) player_.press_attack();  // 공격도 메모장에 (콤보는 취소 구간에서 이어져요)
    if (bindings.pressed(Action::Cast, input)) player_.press_cast();      // 마술도 메모장에 (L / C)
    if (bindings.pressed(Action::Interact, input)) rest_.press_interact();  // E: 쿠션에서 쉬기·일어나기 (13b)
    // F4: 못 가는 칸과 발 상자 표시 켜기/끄기
    if (bindings.pressed(Action::ToggleCollision, input)) show_collision_ = !show_collision_;
}

// 정해진 박자(1/60초)마다 불려요. 캐릭터 이동처럼 "게임 규칙" 계산은 여기서 해요.
void FieldState::fixed_update(double dt_seconds, const engine::InputState& input,
                              const InputBindings& bindings) {
    const engine::Vec2 direction = intent_.direction(bindings, input);
    // 구르기·달리기는 스태미나 규칙에 맞을 때만 돼요 (PlayerMotion이 확인해요).
    const bool run = bindings.held(Action::Run, input);
    // 시뮬레이션 시간 = 박자 × 배수 (멈칫 중이면 0, 쓰러진 직후면 0.3~1). 시간을 바꾸는 유일한 길이에요 (명세 3.2).
    const double sim_dt = dt_seconds * time_scale_.scale();
    if (player_.step(sim_dt, direction, run, map_.map)) {
        // 구르기 시작! 연출(먼지 팍)은 다음 frame_update에서 해요.
        dodge_burst_ = engine::normalized(player_.mover().velocity());
    }
    combat_.step(sim_dt, player_, map_.map);  // 칼 판정, 빛 탄 만들기·날리기, 피해·넉백 주기
    progress_.earn(combat_.runes_earned());     // 쓰러뜨린 몬스터의 룬 (13a)
    items_.step(static_cast<float>(sim_dt), combat_, player_, map_.map);  // 쓰러진 몬스터의 드롭, 자동 줍기 (14b)
    if (combat_.hero_fell()) progress_.fall();  // 쓰러지면 룬 절반을 잃어요 (13a)
    // 멈춤·느림은 진짜 시간으로 줄고, 이번에 누가 맞았으면 멈칫, 주인공이 쓰러졌으면 슬로모션 (12b)
    time_scale_.update(static_cast<float>(dt_seconds), combat_.strongest_hitstop(), combat_.hero_fell());
    combat_fx_.collect(combat_);  // 맞은 일·사라진 빛 탄은 다음 frame_update에서 연출해요 (안 빠지게 모아 둬요)
    rest_.step(static_cast<float>(sim_dt), player_, combat_);  // E: 쿠션에 앉기·일어나기, 쉬기 이동 요청 (13b)
    // 출구를 밟았나 (쓰러졌거나 쉬는 중이면 떠나지 않아요, 12c·13b)
    exits_.step(map_.map, player_.mover().feet(), !player_.dead() && !player_.rest().active());
}

// 화면을 한 장 그릴 때마다 한 번씩 불려요. (1초에 약 60번)
void FieldState::frame_update(double dt_seconds, double alpha, float hour) {
    alpha_ = alpha;
    scene_seconds_ += static_cast<float>(dt_seconds);  // 물 반짝임·풀 흔들림 시계
    const auto dt = static_cast<float>(dt_seconds);
    const engine::Vec2 feet = drawn_feet();

    // 먼지: 구르기 시작이면 확 뿌리고, 걷는 중이면 조금씩
    if (dodge_burst_) {
        dust_emitter_.dodge_burst(feet, *dodge_burst_, fx_random_, ground_fx_);
        dodge_burst_.reset();
    }
    // 걷거나 달리면 먼지가 나요. 먼지는 간 거리마다 나서, 빨리 달리면 더 자주 나요.
    if (player_.mover().state() == Mover::State::Walking || player_.mover().state() == Mover::State::Running) {
        dust_emitter_.walk(feet, player_.mover().velocity(), dt, fx_random_, ground_fx_);
    }
    ground_fx_.update(dt);
    // 분위기 효과(꽃잎·반딧불)는 카메라가 보는 곳 근처에서 계속 생겨나요.
    ambient_.update(dt, camera_rig_.camera().view_rect(), fx_random_, air_fx_);
    // 조명: 깜빡임 시계, 주인공 빛 위치, 횃불 불티
    // 하늘과 공기: 시간 색, 날씨(빗줄기·눈·물 튀김), 밤 효과. 그 결과를 조명에 알려 줘요.
    atmosphere_.update(dt, hour, camera_rig_.camera().view_rect(), fx_random_, air_fx_, ground_fx_);
    lighting_.set_conditions(atmosphere_.conditions().ambient, atmosphere_.conditions().night);
    // 싸움 연출: 번쩍임·흔들림·숫자·불꽃, HP 바, 칼자국, 손끝 빛·빛 탄 (결과를 보기만 해요)
    combat_fx_.update(dt, static_cast<float>(alpha), combat_, player_, feet, fx_random_, air_fx_);
    cushion_view_.update(dt, rest_.resting_cushion(), player_.rest(), fx_random_, air_fx_);  // 피어나기, 하트·z (13b)
    auto moving_lights = combat_fx_.lights(combat_, static_cast<float>(alpha));  // 빛 탄 빛 + 쿠션 초록 빛 (13b)
    for (const auto& light : cushion_view_.lights()) moving_lights.push_back(light);
    for (const auto& light : items_.lights()) moving_lights.push_back(light);  // 바닥 아이템 빛 (14b)
    items_.update(dt, player_, feet, fx_random_, air_fx_);  // 둥실·반짝·빨려 들기 (14b), 물약 병 (14d)
    lighting_.set_moving_lights(std::move(moving_lights));
    lighting_.update(dt, feet, fx_random_, air_fx_);
    air_fx_.update(dt);

    // 주인공 그림: 동작 그림 넘기기와 구르기 잔상. 쓰러졌으면 흑백·눕기 시계가 흘러요 (12b)
    sprite_.update(dt_seconds, player_.mover(), feet, player_.rest());  // 쿠션에서는 앉기·웅크리기·기지개 (13b)
    death_fx_.update(dt, player_.dead());

    // 카메라: 가는 쪽을 조금 더 보여 주면서 캐릭터 몸 가운데를 따라가고, 흔들림을 얹어요.
    camera_rig_.follow(feet, player_.mover().velocity(), player_.mover().speed(),
                       player_.mover().tuning().max_speed, dt);
    camera_rig_.set_shake(combat_fx_.shake_offset());
}

// 이번 화면에 그릴 것들을 그리기 목록(렌더 큐)에 넣어요. 실제로 그리는 건 렌더러가 해요.
void FieldState::render(engine::RenderQueue& queue) {
    const engine::Vec2 feet = drawn_feet();

    // 1) 맵 타일 (화면에 보이는 칸만)
    tiles_queued_ = engine::queue_tilemap(map_.map, map_.tileset, tileset_texture_, camera_rig_.camera(),
                                          scene_seconds_, queue);

    // 2) 발밑 그림자 (22x8): 바닥 위, 캐릭터 아래 단계에 그려요.
    queue_shadow(camera_rig_.camera(), textures_.shadow(), feet, textures_.shadow_size(), queue);

    // 3) 잔상, 플레이어, 먼지: 모두 발 위치(y)로 줄을 서요.
    // 맞은 뒤 무적이면 깜빡여요 (11c). 쓰러졌으면 깜빡이지 않고 옆으로 누워요 (12b)
    const float blink = player_.dead() ? 1.0f : hero_blink_alpha(player_.hit_reaction());
    sprite_.queue(camera_rig_.camera(), feet, player_.mover().facing(), queue, blink, death_fx_.lying());
    // 허수아비·HP 바·번쩍임, 칼자국, 손끝 빛·빛 탄 (F4면 판정 상자)
    combat_fx_.queue(camera_rig_.camera(), combat_, player_, static_cast<float>(alpha_), show_collision_, queue);
    cushion_view_.queue(camera_rig_.camera(), queue);  // 쿠션과 떠오르는 하트·z (13b)
    items_.queue(camera_rig_.camera(), queue);  // 바닥 아이템과 빨려 드는 아이콘 (14b)
    engine::queue_particles(ground_fx_, camera_rig_.camera(), textures_.particles(), engine::DrawLayer::YSorted, queue);
    engine::queue_particles(air_fx_, camera_rig_.camera(), textures_.particles(), engine::DrawLayer::WorldFx, queue,
                            engine::DrawLayer::Glow);  // 공중 효과는 나뭇잎보다도 위에

    // 4) F4를 켰으면 못 가는 칸을 빨갛게, 발 상자를 하늘색으로 덮어요.
    if (show_collision_) {
        engine::queue_collision_overlay(map_.map, camera_rig_.camera(), queue);
        queue_debug_box(camera_rig_.camera(), body_box(feet, player_.mover().tuning().body), kBodyBoxColor, queue);
    }

    // 5) 안개(공중 효과 맨 위, 조명보다 먼저라 밤에는 어두워요)와 조명 준비물(보이는 빛만, 입히는 건 렌더러)
    atmosphere_.queue(camera_rig_.camera(), textures_.light(), queue);
    lighting_frame_ = &lighting_.build(camera_rig_.camera(), textures_.light());
}

// 쿠션에서 레벨업한 뒤 (13c): 새 능력치의 최대 HP·MP·스태미나(가득 채움)와 공격 보너스를 바로 적용해요.
void FieldState::refresh_hero_stats() {
    const auto derived = progress_.derived();  // 한 곳의 계산 함수
    player_.raise_maximums(derived.max_hp, static_cast<float>(derived.max_mp), static_cast<float>(derived.max_stamina));
    combat_.set_hero_stats(derived);
    cushion_view_.celebrate_level_up(player_.mover().feet());  // 빛기둥 파티클과 번쩍임
}

engine::Vec2 FieldState::drawn_feet() const {
    return player_.mover().interpolated_feet(static_cast<float>(alpha_));
}

// 주인공 몸 가운데가 화면(640x360)의 어디에 보이는지. 아이리스 전환이 이 점을 향해 오므라들어요.
engine::Vec2 FieldState::focus_on_screen() const {
    const float half_tile = static_cast<float>(map_.map.tile_size) * 0.5f;
    return camera_rig_.camera().world_to_screen(drawn_feet() - engine::Vec2{0.0f, half_tile});
}

// 디버그 화면(F3)에 보여 줄 정보를 모아 줘요.
FieldDebugInfo FieldState::debug_info() const {
    const engine::Vec2 feet = player_.mover().feet();
    return {map_.map.map_id,
            feet,
            engine::world_to_tile(feet, map_.map.tile_size),
            player_.mover().facing(),
            to_string(player_.state()),
            player_.mover().speed(),
            tiles_queued_,
            ground_fx_.size() + air_fx_.size(),
            ambient_.count(),
            ground_fx_.dropped() + air_fx_.dropped(),
            lighting_.enabled(),
            lighting_.stats(),
            atmosphere_.outdoors(),
            atmosphere_.conditions().night,
            atmosphere_.weather_name(),
            atmosphere_.weather_intensity(),
            player_.stamina().current(),
            player_.stamina().max(),
            player_.stamina().exhausted(),
            player_.mana().current(),
            player_.mana().max(),
            player_.health().current(),
            player_.health().max(),
            player_.attack().combo_index() + 1,
            combat_.debug_text()};
}

}  // namespace wind::game
