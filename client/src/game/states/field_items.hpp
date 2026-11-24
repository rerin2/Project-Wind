#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 아이템 담당이에요. (14b) FieldState가 300줄을 넘지 않게 아이템 일을 여기 모았어요.
// - 규칙(FieldLoot): 쓰러진 몬스터의 드롭 흩어 놓기, 자동 줍기 (정해진 박자마다)
// - 연출(LootView): 톡 튀기·둥실·반짝, 주인공에게 빨려 들기 (화면 한 장마다, 결과를 보기만 해요)
//   물약 마시기(DrinkView, 14d): 손에 든 병, 꿀꺽 빛 가루
// - 인벤토리 화면 요청(14c·14d): 내려놓기·정렬·물약 쓰기를 다음 박자에 규칙대로 처리해요
// - 단축 칸 1~4 (15f): 키를 누르면 그 칸의 아이템을 물약 쓰기와 같은 규칙으로 써요
// - 알림: 주운 것·가득 차서 못 주운 것을 모아 두면 앱이 가져가 왼쪽 아래 알림 줄에 띄워요.
// 가방·도감·바닥 아이템은 필드 밖(ClientApp)에 있고 여기서는 빌려 써요 (ItemWorld).

#include "engine/camera/camera2d.hpp"
#include "engine/input/input_state.hpp"
#include "game/input/input_bindings.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/items/field_loot.hpp"
#include "game/items/item_world.hpp"
#include "game/items/loot_notice.hpp"
#include "game/presentation/drink_view.hpp"
#include "game/presentation/loot_view.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace wind::game {

class Monster;

class FieldItems {
public:
    // items: 빌려 쓸 가방·도감·바닥 아이템, loot: 이 맵의 바닥 목록, view: 연출 그림, seed: 드롭 주사위 씨앗
    FieldItems(const ItemWorld& items, MapLoot& loot, const LootViewSetup& view, std::uint64_t seed);

    // 맵 몬스터들의 드롭 표가 모두 도감에 있나요? 없으면 이유를 기록하고 false (맵을 열지 않아요)
    [[nodiscard]] static bool drops_known(const std::vector<Monster>& monsters, const items::ItemCatalog& catalog);

    // 인벤토리 화면의 요청 (14c·14d): 다음 박자에 규칙대로 처리해요 (그사이 칸이 바뀌었으면 아무 일도 없어요)
    void request_put_down(std::size_t slot) { requests_.push_back({Request::Kind::PutDown, slot}); }
    void request_sort() { requests_.push_back({Request::Kind::Sort, 0}); }
    // 이 칸의 물약을 마셔요 (14d). 못 마시면(공격 중 등, 이미 가득) 알림만 남기고 가방은 그대로예요
    void request_use(std::size_t slot) { requests_.push_back({Request::Kind::Use, slot}); }
    // 1~4 키 (15f): 눌린 단축 칸마다 "그 칸의 아이템 쓰기"를 다음 박자에 요청해요 (화면 한 장마다, 박자 계산 전에)
    void handle_quick_keys(const engine::InputState& input, const InputBindings& bindings);

    // 정해진 박자마다: 화면 요청 처리 -> 드롭과 줍기 (규칙), 주운 것은 연출·알림으로 모아 둬요
    void step(float dt_seconds, const FieldCombat& combat, PlayerMotion& hero, const world::MapData& map);
    // 화면 한 장마다: 연출 시계. feet: 주인공 발 (그린 위치)
    void update(float dt_seconds, const PlayerMotion& hero, engine::Vec2 feet, engine::ParticleRandom& random,
                engine::ParticlePool& air_fx);
    // 바닥 아이템, 빨려 드는 아이콘, 손에 든 물약 병
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const;
    // 밤에 바닥 아이템이 내는 빛과 꿀꺽 빛
    [[nodiscard]] std::vector<engine::Light> lights() const;
    // 모아 둔 알림을 가져가요 (가져가면 비워요)
    [[nodiscard]] std::vector<LootNotice> take_notices() { return std::exchange(notices_, {}); }
    [[nodiscard]] const MapLoot& loot() const { return loot_.loot(); }

private:
    // 인벤토리 화면에서 온 요청 하나 (14c·14d)
    struct Request {
        enum class Kind : std::uint8_t { PutDown, Sort, Use, Quick };
        Kind kind = Kind::Sort;
        std::size_t slot = 0;  // PutDown·Use일 때 가방 칸, Quick일 때 단축 칸 (0~3)
    };
    // 단축 칸 쓰기 (15f): 그 아이템이 든 가방의 첫 칸을 찾아 물약 쓰기와 같은 규칙으로. 0개면 아무 일도 없어요
    void use_quick(std::size_t quick_slot, PlayerMotion& hero);
    // 물약 쓰기: 확인 -> 가방에서 하나 꺼내기 -> 마시기 시작 (14d)
    void use(std::size_t slot, PlayerMotion& hero);

    ItemWorld items_;                   // 가방·도감·바닥 아이템 (빌려 씀)
    std::vector<Request> requests_;     // 다음 박자에 처리할 화면 요청 (14c)
    FieldLoot loot_;                    // 규칙: 드롭·줍기
    LootView view_;                     // 연출: 바닥 아이템
    DrinkView drink_view_;              // 연출: 물약 마시기 (14d)
    std::vector<LootNotice> notices_;   // 알림 줄에 띄울 일
};

}  // namespace wind::game
