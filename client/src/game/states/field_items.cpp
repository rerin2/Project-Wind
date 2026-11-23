// 이 파일은 무엇을 하나요?
// 필드 아이템 담당: 규칙(FieldLoot)과 연출(LootView)을 차례로 부르고, 알림을 모아요.

#include "game/states/field_items.hpp"

#include "game/items/drop_roll.hpp"
#include "game/monsters/monster.hpp"

#include "wind/items/inventory.hpp"
#include "wind/log.hpp"

namespace wind::game {
namespace {

constexpr float kBodyLift = 10.0f;  // 발에서 몸 가운데까지 (아이템이 빨려 드는 곳)

}  // namespace

FieldItems::FieldItems(const ItemWorld& items, MapLoot& loot, const LootViewSetup& view, std::uint64_t seed)
    : items_(items), loot_(loot, seed), view_(view, items.catalog) {}

bool FieldItems::drops_known(const std::vector<Monster>& monsters, const items::ItemCatalog& catalog) {
    for (const auto& monster : monsters) {
        if (const auto bad = unknown_drop_item(monster.data().drops, catalog)) {
            log::error("Monster {} drops unknown item {}", monster.data().id, *bad);
            return false;
        }
    }
    return true;
}

void FieldItems::step(float dt_seconds, const FieldCombat& combat, const PlayerMotion& hero,
                      const world::MapData& map) {
    for (const auto& request : requests_) {  // 인벤토리 화면 요청부터 (14c)
        if (request.kind == Request::Kind::PutDown) (void)loot_.put_down(request.slot, hero, map, items_.bag);
        if (request.kind == Request::Kind::Sort) (void)items_.bag.sort(items_.catalog);  // 칸이 모자랄 일은 없어요
    }
    requests_.clear();
    loot_.step(dt_seconds, combat, hero, map, items_.bag, items_.catalog);
    view_.collect(loot_.picked());  // 주운 것은 빨려 드는 연출로
    for (const auto& item : loot_.picked()) notices_.push_back({item.stack, false});  // "슬라임 젤리 ×2"
    for (const auto& stack : loot_.refused()) notices_.push_back({stack, true});      // "가방이 가득 찼다"
}

void FieldItems::update(float dt_seconds, engine::Vec2 feet, engine::ParticleRandom& random,
                        engine::ParticlePool& air_fx) {
    view_.update(dt_seconds, feet - engine::Vec2{0.0f, kBodyLift}, random, air_fx);
}

}  // namespace wind::game
