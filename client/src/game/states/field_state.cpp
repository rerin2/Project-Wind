// 이 파일은 무엇을 하나요?
// "필드 화면"(맵 위를 돌아다니는 화면)이 실제로 어떻게 움직이는지 적은 곳이에요.
// 맵과 캐릭터 그림을 불러오고, 키를 보고 캐릭터를 옮기고, 먼지·잔상·그림자를 만들고,
// 카메라를 따라가게 하고, 화면에 그릴 것들을 "그리기 목록"(렌더 큐)에 넣어요.

#include "game/states/field_state.hpp"

#include "engine/animation/sprite_sheet_loader.hpp"
#include "engine/fx/particle_drawing.hpp"
#include "engine/map/coordinates.hpp"
#include "engine/map/tilemap_renderer.hpp"
#include "engine/render/renderer.hpp"
#include "game/physics/tile_collision.hpp"

#include "wind/log.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>
#include <utility>

namespace wind::game {
namespace {

// 카메라가 플레이어를 따라가는 속도 (클수록 빨리 따라붙어요)
constexpr float kCameraStiffness = 8.0f;
// 카메라 앞보기: 최대 몇 픽셀 앞을 보여 줄지, 얼마나 빨리 따라갈지
constexpr float kLookAheadDistance = 28.0f;
constexpr float kLookAheadSharpness = 3.0f;
// 구르기 버튼을 몇 초 동안 기억해 줄지 (명세 5.8 입력 버퍼 기본값)
constexpr double kInputBufferSeconds = 0.15;
// 파티클 통 크기 (이보다 많으면 버리고 세요, 명세 6.6)
constexpr std::size_t kGroundFxCapacity = 1024;  // 땅 효과 (먼지)
constexpr std::size_t kAirFxCapacity = 4000;     // 공중 효과 (꽃잎, 반딧불, 시험용 부하)
// 파티클 주사위 씨앗. 연출용이라 아무 숫자나 괜찮지만, 고정하면 매번 같은 모양이 나와요.
constexpr std::uint32_t kFxSeed = 20261109;
// 걷기 그림 넘기는 속도: 최고 속도일 때 1배, 느리면 천천히 (최소 0.5배), 구를 때는 2배
constexpr double kMinWalkAnimRate = 0.5;
constexpr double kDodgeAnimRate = 2.0;

// 발밑 그림자 크기와 진하기 (가운데가 가장 진하고 가장자리는 투명해요)
constexpr std::int32_t kShadowWidth = 22;
constexpr std::int32_t kShadowHeight = 8;
constexpr engine::Color kShadowColor{0, 0, 0, 110};
// 파티클 그림: 흰색이라 효과 파일의 색을 곱해서 써요.
constexpr std::int32_t kSoftParticleSize = 8;
constexpr engine::Color kWhite{255, 255, 255, 255};
// 빛 그림 크기 (확대해서 쓰니까 너무 크지 않아도 돼요)
constexpr std::int32_t kLightTextureSize = 128;
// F4: 발 상자 표시 색 (반투명 하늘색)
constexpr engine::Color kBodyBoxColor{80, 200, 255, 140};

// 플레이어 그림 묶음(스프라이트 시트)에 꼭 있어야 하는 동작 그림(클립) 이름
constexpr std::string_view kIdleClip = "idle";  // 가만히 서 있기
constexpr std::string_view kWalkClip = "walk";  // 걷기

// 시작 칸에서 발을 둘 위치: 칸의 가로 가운데, 세로는 칸의 3/4 지점.
// 발 상자(높이 8)가 칸 안에 쏙 들어가서 시작하자마자 벽에 걸리지 않아요.
engine::Vec2 spawn_feet(world::TileCoord tile, std::int32_t tile_size) {
    const auto size = static_cast<float>(tile_size);
    return {static_cast<float>(tile.x) * size + size * 0.5f,
            static_cast<float>(tile.y) * size + size * 0.75f};
}

std::string_view state_name(Mover::State state) {
    switch (state) {
        case Mover::State::Idle: return "idle";
        case Mover::State::Walking: return "walking";
        case Mover::State::Dodging: return "dodging";
    }
    return "?";
}

}  // namespace

// 필드 화면 만들기: 준비물을 하나씩 불러와요. 하나라도 잘못되면 nullptr(없음)를 돌려줘요.
std::unique_ptr<FieldState> FieldState::create(engine::ResourceManager& resources,
                                               engine::Renderer& renderer,
                                               std::string_view map_path,
                                               std::string_view player_sheet_path,
                                               engine::Vec2 viewport,
                                               std::string_view spawn_id, FieldStress stress) {
    // 1) 맵 파일(과 맵이 쓰는 타일셋)을 읽고 검사해요.
    auto map = content::load_map_file(resources.content_root(), map_path);
    if (!map.ok()) {
        content::log_content_errors("map", map.errors);
        return nullptr;
    }
    // 2) 캐릭터 그림 설명서(스프라이트 시트 JSON)를 읽고 검사해요.
    auto sheet = engine::load_sprite_sheet_file(resources.content_root(), player_sheet_path);
    if (!sheet.ok()) {
        content::log_content_errors("player sprite sheet", sheet.errors);
        return nullptr;
    }
    // 3) "서 있기"와 "걷기" 그림이 둘 다 있어야 해요.
    if (sheet.value->clip(kIdleClip) == nullptr || sheet.value->clip(kWalkClip) == nullptr) {
        log::error("Player sprite sheet '{}' needs '{}' and '{}' clips", player_sheet_path,
                   kIdleClip, kWalkClip);
        return nullptr;
    }

    // 4) 등장 자리: 요청한 이름이 있으면 거기. "default"를 요청했는데 없으면 첫 번째 자리.
    //    다른 이름(출구가 가리킨 자리)이 없으면 맵 자료가 틀린 거라 들어가지 않아요.
    const auto& spawns = map.value->map;
    const world::SpawnPoint* spawn = spawns.find_spawn(spawn_id);
    if (spawn == nullptr && spawn_id == "default") {
        spawn = &spawns.spawn_points.front();
    }
    if (spawn == nullptr) {
        log::error("Map '{}' has no spawn point '{}'", map_path, spawn_id);
        return nullptr;
    }
    const world::TileCoord spawn_tile = spawn->position;

    // 4) 그림 파일(텍스처)을 불러와요. 그림이 없어도 체크무늬 대신 그림으로 계속 진행해요.
    //    (무엇이 없는지는 리소스 관리자가 기록해 줘요.)
    const auto tileset_texture = resources.texture(map.value->tileset.image);
    const auto player_texture = resources.texture(sheet.value->image);
    // 5) 그림자와 파티클 그림은 파일 없이 코드로 만들어요. 실패해도 그 효과만 안 보일 뿐 계속해요.
    const EffectTextures textures{
        renderer.create_soft_ellipse_texture(kShadowWidth, kShadowHeight, kShadowColor),
        {renderer.create_soft_ellipse_texture(kSoftParticleSize, kSoftParticleSize, kWhite),
         renderer.create_solid_texture(1, 1, kWhite)},
        renderer.create_light_texture(kLightTextureSize),
    };

    // 6) 파티클 효과 요리법을 읽어요: 먼지, 맵 분위기 효과, 빛 자리 효과 (잘못된 파일은 건너뛰어요)
    FieldEffects effects =
        load_field_effects(resources.content_root(), map.value->map.ambience, stress.particles);
    if (stress.lights > 0) {
        log::info("Light stress test: {} extra lights", stress.lights);
    }

    const auto& ambience = map.value->map.ambience;
    log::info("Entered map '{}' ({}x{} tiles, {} ambient effect(s), {} light(s), lighting {})",
              map.value->map.map_id, map.value->map.width, map.value->map.height,
              effects.ambient.size(), ambience.lights.size(),
              ambience.ambient_light ? "on" : "off");
    return std::unique_ptr<FieldState>(new FieldState(renderer, std::move(*map.value),
                                                      std::move(*sheet.value),
                                                      tileset_texture, player_texture, textures,
                                                      std::move(effects), viewport, spawn_tile,
                                                      stress.lights));
}

// 준비물을 받아서 필드 화면을 실제로 꾸려요.
FieldState::FieldState(engine::Renderer& renderer, content::LoadedMap map, engine::SpriteSheet sheet,
                       engine::TextureHandle tileset_texture, engine::TextureHandle player_texture,
                       EffectTextures textures, FieldEffects effects, engine::Vec2 viewport,
                       world::TileCoord spawn_tile, std::uint32_t stress_lights)
    : renderer_(&renderer),
      map_(std::move(map)),
      sheet_(std::move(sheet)),
      tileset_texture_(tileset_texture),
      player_texture_(player_texture),
      textures_(textures),
      effects_(std::move(effects)),
      look_ahead_(kLookAheadDistance, kLookAheadSharpness),
      // 캐릭터가 처음 설 곳 (create()가 고른 등장 자리)
      player_(spawn_feet(spawn_tile, map_.map.tile_size),
              world::Direction::Down, MovementTuning{}),
      buffer_(kInputBufferSeconds),
      fx_random_(kFxSeed),
      ground_fx_(kGroundFxCapacity),
      air_fx_(kAirFxCapacity),
      // effects_는 위에서 이미 만들어졌고, 이 화면이 사라질 때까지 그대로라서 빌려 써도 안전해요.
      dust_emitter_(effects_.dust_walk ? &*effects_.dust_walk : nullptr,
                    effects_.dust_dodge ? &*effects_.dust_dodge : nullptr),
      // 조명: 맵 분위기 + (시험용이면) 시작 위치 주변에 빛을 더 놓아요.
      lighting_(map_.map.ambience, map_.map.tile_size, effects_.light_fx, stress_lights,
                player_.feet()) {
    for (const auto& effect : effects_.ambient) {
        ambient_.add(effect);
    }
    // 카메라 준비: 화면 크기를 알려 주고, 맵 밖은 보여 주지 않도록 맵 크기를 경계로 정해요.
    const auto size = static_cast<float>(map_.map.tile_size);
    camera_.set_viewport(viewport);
    camera_.set_world_bounds({0.0f, 0.0f, static_cast<float>(map_.map.width) * size,
                              static_cast<float>(map_.map.height) * size});
    // 카메라는 캐릭터 몸 가운데(발에서 반 칸 위)를 봐요. 시작할 때는 바로 맞춰요.
    camera_.center_on(player_.feet() - engine::Vec2{0.0f, size * 0.5f});
    animator_.play(sheet_.clip(kIdleClip));  // 처음엔 서 있는 모습
}

FieldState::~FieldState() {
    // 파일에서 읽은 그림(타일, 캐릭터)은 리소스 관리자가 정리해요. 여기서는 코드로 만든 것만.
    for (const engine::TextureHandle texture :
         {textures_.shadow, textures_.particles.soft, textures_.particles.pixel, textures_.light}) {
        renderer_->destroy_texture(texture);
    }
}

void FieldState::handle_input(const engine::InputState& input, const InputBindings& bindings) {
    intent_.update(bindings, input);  // 새로 누른 방향키를 기억
    if (bindings.pressed(Action::Dodge, input)) {
        // 바로 구르지 않고 메모장에 적어 둬요. 다음 박자 계산 때 꺼내 써요.
        // (이번 화면에 박자 계산이 0번이어도 버튼이 사라지지 않아요)
        buffer_.press(Action::Dodge);
    }
    if (bindings.pressed(Action::ToggleCollision, input)) {
        show_collision_ = !show_collision_;  // F4: 못 가는 칸과 발 상자 표시 켜기/끄기
    }
}

// 정해진 박자(1/60초)마다 불려요. 캐릭터 이동처럼 "게임 규칙" 계산은 여기서 해요.
void FieldState::fixed_update(double dt_seconds, const engine::InputState& input,
                              const InputBindings& bindings) {
    const engine::Vec2 direction = intent_.direction(bindings, input);
    // 구를 수 있을 때만 메모를 꺼내요. 못 구르면 메모는 남아 있다가 0.15초 뒤 사라져요.
    const bool dodge = player_.can_dodge() && buffer_.consume(Action::Dodge);
    if (player_.step(dt_seconds, direction, dodge, map_.map)) {
        // 구르기 시작! 연출(먼지 팍)은 다음 frame_update에서 해요.
        dodge_burst_ = engine::normalized(player_.velocity());
    }
    buffer_.tick(dt_seconds);

    // 출구 확인: 발이 출구 칸에 들어가면 "다른 맵으로 가 주세요" 요청을 남겨요.
    // 출구가 아닌 칸을 한 번 밟은 뒤에만 출구가 동작해요(무장). 그래야 출구 바로 옆에서
    // 나타났을 때 곧장 되돌아가는 일이 없어요.
    const world::MapExit* exit = map_.map.exit_at(engine::world_to_tile(player_.feet(), map_.map.tile_size));
    if (exit == nullptr) {
        exit_armed_ = true;
    } else if (exit_armed_ && !exit_request_) {
        exit_request_ = *exit;
        exit_armed_ = false;
    }
}

// 화면을 한 장 그릴 때마다 한 번씩 불려요. (1초에 약 60번)
void FieldState::frame_update(double dt_seconds, double alpha) {
    alpha_ = alpha;
    const auto dt = static_cast<float>(dt_seconds);
    const engine::Vec2 feet = drawn_feet();

    update_animation(dt_seconds);

    // 먼지: 구르기 시작이면 확 뿌리고, 걷는 중이면 조금씩
    if (dodge_burst_) {
        dust_emitter_.dodge_burst(feet, *dodge_burst_, fx_random_, ground_fx_);
        dodge_burst_.reset();
    }
    if (player_.state() == Mover::State::Walking) {
        dust_emitter_.walk(feet, player_.velocity(), dt, fx_random_, ground_fx_);
    }
    ground_fx_.update(dt);
    // 분위기 효과(꽃잎·반딧불)는 카메라가 보는 곳 근처에서 계속 생겨나요.
    ambient_.update(dt, camera_.view_rect(), fx_random_, air_fx_);
    // 조명: 깜빡임 시계, 주인공 빛 위치, 횃불 불티
    lighting_.update(dt, feet, fx_random_, air_fx_);
    air_fx_.update(dt);

    // 잔상: 구르는 동안 사진을 찍어요
    trail_.update(dt, player_.state() == Mover::State::Dodging, feet,
                  sheet_.frame_rect(player_.facing(), animator_.column()));

    // 카메라: 가는 쪽을 조금 더 보여 주면서 캐릭터 몸 가운데를 따라가요.
    const float max_speed = player_.tuning().max_speed;
    const float run = max_speed > 0.0f ? std::min(1.0f, player_.speed() / max_speed) : 0.0f;
    look_ahead_.update(engine::normalized(player_.velocity()) * run, dt);
    const float half_tile = static_cast<float>(map_.map.tile_size) * 0.5f;
    camera_.follow(feet - engine::Vec2{0.0f, half_tile} + look_ahead_.offset(), dt,
                   kCameraStiffness);
}

void FieldState::update_animation(double dt_seconds) {
    double rate = 1.0;
    switch (player_.state()) {
        case Mover::State::Idle:
            animator_.play(sheet_.clip(kIdleClip));
            break;
        case Mover::State::Walking: {
            animator_.play(sheet_.clip(kWalkClip));
            // 천천히 걸으면 발도 천천히 움직여요 (발이 미끄러지는 것처럼 보이지 않게)
            const double max_speed = player_.tuning().max_speed;
            rate = max_speed > 0.0 ? std::max(kMinWalkAnimRate, player_.speed() / max_speed) : 1.0;
            break;
        }
        case Mover::State::Dodging:
            // 구르기 그림은 아직 없어서, 걷기 그림을 두 배 빨리 넘겨요.
            animator_.play(sheet_.clip(kWalkClip));
            rate = kDodgeAnimRate;
            break;
    }
    animator_.update(dt_seconds * rate);
}

// 이번 화면에 그릴 것들을 그리기 목록(렌더 큐)에 넣어요. 실제로 그리는 건 렌더러가 해요.
void FieldState::render(engine::RenderQueue& queue) {
    const engine::Vec2 feet = drawn_feet();

    // 1) 맵 타일 (화면에 보이는 칸만)
    tiles_queued_ = engine::queue_tilemap(map_.map, map_.tileset, tileset_texture_, camera_, queue);

    // 2) 발밑 그림자: 바닥 위, 캐릭터 아래 단계에 그려요.
    if (textures_.shadow.valid()) {
        const engine::Vec2 screen = camera_.world_to_screen(feet);
        engine::SpriteDraw shadow;
        shadow.texture = textures_.shadow;
        shadow.dest = {std::round(screen.x - static_cast<float>(kShadowWidth) * 0.5f),
                       std::round(screen.y - static_cast<float>(kShadowHeight) * 0.5f),
                       static_cast<float>(kShadowWidth), static_cast<float>(kShadowHeight)};
        queue.push_sprite(engine::DrawLayer::Shadows, feet.y, shadow);
    }

    // 3) 잔상, 플레이어, 먼지: 모두 발 위치(y)로 줄을 서요.
    trail_.queue(camera_, player_texture_, sheet_.origin, {sheet_.frame_width, sheet_.frame_height},
                 queue);
    queue_player(queue, feet);
    engine::queue_particles(ground_fx_, camera_, textures_.particles, engine::DrawLayer::YSorted, queue);
    // 공중 효과는 나뭇잎보다도 위에 그려요.
    engine::queue_particles(air_fx_, camera_, textures_.particles, engine::DrawLayer::WorldFx, queue);

    // 4) F4를 켰으면 못 가는 칸을 빨갛게, 발 상자를 하늘색으로 덮어요.
    if (show_collision_) {
        engine::queue_collision_overlay(map_.map, camera_, queue);
        queue_body_box(queue, feet);
    }

    // 5) 조명 준비물: 화면에 보이는 빛만 골라요. 실제로 입히는 건 렌더러가 해요.
    lighting_frame_ = &lighting_.build(camera_, textures_.light);
}

void FieldState::queue_player(engine::RenderQueue& queue, engine::Vec2 feet) const {
    // 그림 속 "발 위치"(origin)가 맵 위의 진짜 발 위치에 딱 오도록 그려요.
    const engine::Vec2 screen = camera_.world_to_screen(feet);  // 맵 위치 -> 화면 위치
    engine::SpriteDraw sprite;
    sprite.texture = player_texture_;
    // 그림 묶음에서 "지금 방향, 지금 동작 장면"에 해당하는 부분을 오려 와요.
    sprite.source = sheet_.frame_rect(player_.facing(), animator_.column());
    // 위치는 반올림해서 정수 픽셀에 맞춰요. 그래야 픽셀 그림이 흐릿하게 떨리지 않아요.
    sprite.dest = {std::round(screen.x - static_cast<float>(sheet_.origin.x)),
                   std::round(screen.y - static_cast<float>(sheet_.origin.y)),
                   static_cast<float>(sheet_.frame_width), static_cast<float>(sheet_.frame_height)};
    // 발 위치(y)로 줄을 세워서, 나무 밑동보다 아래(앞)에 서면 나무보다 앞에 그려져요.
    queue.push_sprite(engine::DrawLayer::YSorted, feet.y, sprite);
}

void FieldState::queue_body_box(engine::RenderQueue& queue, engine::Vec2 feet) const {
    const engine::RectF box = body_box(feet, player_.tuning().body);
    const engine::Vec2 top_left = camera_.world_to_screen({box.x, box.y});
    queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, {top_left.x, top_left.y, box.w, box.h},
                    kBodyBoxColor);
}

engine::Vec2 FieldState::drawn_feet() const {
    return player_.interpolated_feet(static_cast<float>(alpha_));
}

// 디버그 화면(F3)에 보여 줄 정보를 모아 줘요.
FieldDebugInfo FieldState::debug_info() const {
    const engine::Vec2 feet = player_.feet();
    return {map_.map.map_id,
            feet,
            engine::world_to_tile(feet, map_.map.tile_size),
            player_.facing(),
            state_name(player_.state()),
            player_.speed(),
            tiles_queued_,
            ground_fx_.size() + air_fx_.size(),
            ambient_.count(),
            ground_fx_.dropped() + air_fx_.dropped(),
            lighting_.enabled(),
            lighting_.stats()};
}

}  // namespace wind::game
