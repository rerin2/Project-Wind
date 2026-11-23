#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 아이템 담당이에요. (14b) FieldState가 300줄을 넘지 않게 아이템 일을 여기 모았어요.
// - 규칙(FieldLoot): 쓰러진 몬스터의 드롭 흩어 놓기, 자동 줍기 (정해진 박자마다)
// - 연출(LootView): 톡 튀기·둥실·반짝, 주인공에게 빨려 들기 (화면 한 장마다, 결과를 보기만 해요)
// - 알림: 주운 것·가득 차서 못 주운 것을 모아 두면 앱이 가져가 왼쪽 아래 알림 줄에 띄워요.
// 가방·도감·바닥 아이템은 필드 밖(ClientApp)에 있고 여기서는 빌려 써요 (ItemWorld).

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/items/field_loot.hpp"
#include "game/items/item_world.hpp"
#include "game/items/loot_notice.hpp"
#include "game/presentation/loot_view.hpp"

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

    // 정해진 박자마다: 드롭과 줍기 (규칙), 주운 것은 연출·알림으로 모아 둬요
    void step(float dt_seconds, const FieldCombat& combat, const PlayerMotion& hero, const world::MapData& map);
    // 화면 한 장마다: 연출 시계. feet: 주인공 발 (그린 위치)
    void update(float dt_seconds, engine::Vec2 feet, engine::ParticleRandom& random, engine::ParticlePool& air_fx);
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const { view_.queue(camera, loot_.loot(), queue); }
    // 밤에 바닥 아이템이 내는 빛
    [[nodiscard]] std::vector<engine::Light> lights() const { return view_.lights(loot_.loot()); }
    // 모아 둔 알림을 가져가요 (가져가면 비워요)
    [[nodiscard]] std::vector<LootNotice> take_notices() { return std::exchange(notices_, {}); }
    [[nodiscard]] const MapLoot& loot() const { return loot_.loot(); }

private:
    ItemWorld items_;                   // 가방·도감·바닥 아이템 (빌려 씀)
    FieldLoot loot_;                    // 규칙: 드롭·줍기
    LootView view_;                     // 연출
    std::vector<LootNotice> notices_;   // 알림 줄에 띄울 일
};

}  // namespace wind::game
