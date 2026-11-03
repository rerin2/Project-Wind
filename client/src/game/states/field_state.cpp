// 이 파일은 무엇을 하나요?
// "필드 화면"(맵 위를 걸어 다니는 화면)이 실제로 어떻게 움직이는지 적은 곳이에요.
// 맵과 캐릭터 그림을 불러오고, 키를 보고 캐릭터를 옮기고, 카메라를 따라가게 하고,
// 화면에 그릴 것들을 "그리기 목록"(렌더 큐)에 넣어요.

#include "game/states/field_state.hpp"

#include "engine/animation/sprite_sheet_loader.hpp"
#include "engine/map/coordinates.hpp"
#include "engine/map/tilemap_renderer.hpp"

#include "wind/log.hpp"

#include <cmath>
#include <utility>

namespace wind::game {
namespace {

// 한 칸 이동 시간(0.2초 = 1초에 5칸). 나중에는 캐릭터의 이동 속도 능력치(movement_rate)로
// 데이터에서 정하게 바꿀 거예요.
constexpr double kStepSeconds = 0.2;
// 카메라가 플레이어를 따라가는 속도 (클수록 빨리 따라붙어요)
constexpr float kCameraStiffness = 10.0f;
// 플레이어 그림 묶음(스프라이트 시트)에 꼭 있어야 하는 동작 그림(클립) 이름
constexpr std::string_view kIdleClip = "idle";  // 가만히 서 있기
constexpr std::string_view kWalkClip = "walk";  // 걷기

// 불러오기에 실패했을 때 무엇이 왜 틀렸는지 한 줄씩 기록(로그)에 남겨요.
void log_errors(std::string_view what, const std::vector<content::ContentError>& errors) {
    log::error("Failed to load {} ({} error(s)):", what, errors.size());
    for (const auto& error : errors) {
        log::error("  {}", content::to_string(error));
    }
}

}  // namespace

// 필드 화면 만들기: 준비물을 하나씩 불러와요. 하나라도 잘못되면 nullptr(없음)를 돌려줘요.
std::unique_ptr<FieldState> FieldState::create(engine::ResourceManager& resources,
                                               std::string_view map_path,
                                               std::string_view player_sheet_path,
                                               engine::Vec2 viewport) {
    // 1) 맵 파일(과 맵이 쓰는 타일셋)을 읽고 검사해요.
    auto map = content::load_map_file(resources.content_root(), map_path);
    if (!map.ok()) {
        log_errors("map", map.errors);
        return nullptr;
    }
    // 2) 캐릭터 그림 설명서(스프라이트 시트 JSON)를 읽고 검사해요.
    auto sheet = engine::load_sprite_sheet_file(resources.content_root(), player_sheet_path);
    if (!sheet.ok()) {
        log_errors("player sprite sheet", sheet.errors);
        return nullptr;
    }
    // 3) "서 있기"와 "걷기" 그림이 둘 다 있어야 해요.
    if (sheet.value->clip(kIdleClip) == nullptr || sheet.value->clip(kWalkClip) == nullptr) {
        log::error("Player sprite sheet '{}' needs '{}' and '{}' clips", player_sheet_path,
                   kIdleClip, kWalkClip);
        return nullptr;
    }

    // 4) 그림 파일(텍스처)을 불러와요. 그림이 없어도 체크무늬 대신 그림으로 계속 진행해요.
    //    (무엇이 없는지는 리소스 관리자가 기록해 줘요.)
    const auto tileset_texture = resources.texture(map.value->tileset.image);
    const auto player_texture = resources.texture(sheet.value->image);

    log::info("Entered map '{}' ({}x{} tiles)", map.value->map.map_id, map.value->map.width,
              map.value->map.height);
    return std::unique_ptr<FieldState>(new FieldState(std::move(*map.value), std::move(*sheet.value),
                                                      tileset_texture, player_texture, viewport));
}

// 준비물을 받아서 필드 화면을 실제로 꾸려요.
FieldState::FieldState(content::LoadedMap map, engine::SpriteSheet sheet,
                       engine::TextureHandle tileset_texture, engine::TextureHandle player_texture,
                       engine::Vec2 viewport)
    : map_(std::move(map)),
      sheet_(std::move(sheet)),
      tileset_texture_(tileset_texture),
      player_texture_(player_texture),
      // 캐릭터가 처음 설 곳(스폰 지점): "default"라는 이름이 있으면 거기, 없으면 첫 번째 지점.
      // (맵 검사기는 "default"가 꼭 있다고 약속하지 않아서 이렇게 해요.)
      player_(map_.map.find_spawn("default") != nullptr ? map_.map.find_spawn("default")->position
                                                        : map_.map.spawn_points.front().position,
              world::Direction::Down, kStepSeconds) {
    // 카메라 준비: 화면 크기를 알려 주고, 맵 밖은 보여 주지 않도록 맵 크기를 경계로 정해요.
    const auto size = static_cast<float>(map_.map.tile_size);
    camera_.set_viewport(viewport);
    camera_.set_world_bounds({0.0f, 0.0f, static_cast<float>(map_.map.width) * size,
                              static_cast<float>(map_.map.height) * size});
    camera_.center_on(player_feet());  // 시작할 때는 캐릭터에게 바로 카메라를 맞춰요
    animator_.play(sheet_.clip(kIdleClip));  // 처음엔 서 있는 모습
}

// 화면을 한 장 그릴 때마다 한 번씩 불려요. (1초에 약 60번)
void FieldState::frame_update(double dt_seconds, const engine::InputState& input,
                              const InputBindings& bindings) {
    intent_.update(bindings, input);  // 새로 누른 방향키를 기억
    if (bindings.pressed(Action::ToggleCollision, input)) {
        show_collision_ = !show_collision_;  // F4: 못 가는 칸 표시 켜기/끄기
    }

    // 이동 중이거나 이동 키를 누르고 있으면 걷기, 아니면 서 있기 그림
    const bool walking = player_.moving() || intent_.direction(bindings, input).has_value();
    animator_.play(sheet_.clip(walking ? kWalkClip : kIdleClip));
    animator_.update(dt_seconds);  // 그림을 다음 장으로 넘길 때가 됐는지 확인

    // 카메라는 캐릭터 몸 가운데(발에서 반 칸 위)를 화면 가운데에 두려고 천천히 따라가요.
    const float half_tile = static_cast<float>(map_.map.tile_size) * 0.5f;
    camera_.follow(player_feet() - engine::Vec2{0.0f, half_tile}, static_cast<float>(dt_seconds),
                   kCameraStiffness);
}

// 정해진 박자(1/60초)마다 불려요. 캐릭터 이동처럼 "게임 규칙" 계산은 여기서 해요.
void FieldState::fixed_update(double dt_seconds, const engine::InputState& input,
                              const InputBindings& bindings) {
    const double leftover = player_.update(dt_seconds);  // 이동 진행. 막 도착했으면 남은 시간이 와요
    // 이동이 끝났는데 방향키를 계속 누르고 있으면, 남은 시간을 이어 써서
    // 멈칫하지 않고 바로 다음 칸으로 출발해요.
    if (!player_.moving()) {
        if (const auto dir = intent_.direction(bindings, input)) {
            if (player_.try_move(*dir, map_.map) == GridMover::MoveResult::Started) {
                player_.update(leftover);
            }
        }
    }
}

// 이번 화면에 그릴 것들을 그리기 목록(렌더 큐)에 넣어요. 실제로 그리는 건 렌더러가 해요.
void FieldState::render(engine::RenderQueue& queue) {
    // 1) 맵 타일 (화면에 보이는 칸만)
    tiles_queued_ = engine::queue_tilemap(map_.map, map_.tileset, tileset_texture_, camera_, queue);

    // 2) 플레이어: 그림 속 "발 위치"(origin)가 맵 위의 진짜 발 위치에 딱 오도록 그려요.
    const engine::Vec2 feet = player_feet();
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

    // 3) F4를 켰으면 못 가는 칸을 빨갛게 덮어요.
    if (show_collision_) {
        engine::queue_collision_overlay(map_.map, camera_, queue);
    }
}

// 디버그 화면(F3)에 보여 줄 정보를 모아 줘요.
FieldDebugInfo FieldState::debug_info() const {
    return {map_.map.map_id, player_.tile(), player_.facing(), player_.moving(), tiles_queued_};
}

engine::Vec2 FieldState::player_feet() const {
    // 발은 칸의 가로 한가운데, 칸의 아래쪽 끝선에 놓여요.
    // visual_tile()은 걸어가는 중이면 1.5 같은 중간 값이라 발도 스르륵 움직여요.
    const auto size = static_cast<float>(map_.map.tile_size);
    const engine::Vec2 tile = player_.visual_tile();
    return {tile.x * size + size * 0.5f, tile.y * size + size};
}

}  // namespace wind::game
