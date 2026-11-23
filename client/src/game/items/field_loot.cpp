// 이 파일은 무엇을 하나요?
// 드롭 흩어 놓기와 자동 줍기. 수치의 이유는 docs/GAMEPLAY.md "Drops and pickup" 절에 있어요.

#include "game/items/field_loot.hpp"

#include "engine/map/coordinates.hpp"
#include "game/entities/player_motion.hpp"
#include "game/items/drop_roll.hpp"
#include "game/items/map_loot.hpp"
#include "game/states/field_combat.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/log.hpp"
#include "wind/world/map.hpp"

#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

constexpr float kScatterMin = 10.0f;  // 쓰러진 자리에서 이만큼 ~
constexpr float kScatterMax = 20.0f;  // 이만큼 떨어진 곳에 떨어져요 (픽셀)

}  // namespace

FieldLoot::FieldLoot(MapLoot& loot, std::uint64_t seed) : loot_(loot), random_(seed) {}

void FieldLoot::step(float dt_seconds, const FieldCombat& combat, const PlayerMotion& hero,
                     const world::MapData& map, items::Inventory& bag, const items::ItemCatalog& catalog) {
    picked_.clear();
    refused_.clear();
    spawn_drops(combat, map);   // 1) 쓰러진 몬스터의 드롭
    loot_.age(dt_seconds);      // 2) 튀어 오르는 시간 재기
    if (!hero.dead() && !hero.rest().active()) pick_up(hero, bag, catalog);  // 3) 줍기
}

void FieldLoot::spawn_drops(const FieldCombat& combat, const world::MapData& map) {
    for (const auto& defeat : combat.rune_drops()) {  // 쓰러진 몬스터마다 한 번씩 와요 (룬 0이어도)
        const auto& table = combat.monsters()[defeat.monster].data().drops;
        for (const auto& stack : roll_drops(table, random_)) {
            // 둘레 아무 방향으로 조금 떨어진 곳. 벽·물이면 쓰러진 자리 그대로
            const float angle = random_.next_unit() * 2.0f * std::numbers::pi_v<float>;
            const float distance = kScatterMin + random_.next_unit() * (kScatterMax - kScatterMin);
            engine::Vec2 at = defeat.at + engine::Vec2{std::cos(angle), std::sin(angle)} * distance;
            if (!map.is_walkable(engine::world_to_tile(at, map.tile_size))) at = defeat.at;
            loot_.drop(stack, defeat.at, at);
            log::debug("Dropped {} x{} at ({:.0f}, {:.0f})", stack.item_id, stack.count, at.x, at.y);
        }
    }
}

void FieldLoot::pick_up(const PlayerMotion& hero, items::Inventory& bag, const items::ItemCatalog& catalog) {
    const engine::Vec2 feet = hero.mover().feet();
    std::vector<std::uint32_t> taken;  // 이번에 주운 번호 (목록을 도는 중에는 지우지 않아요)
    for (auto& item : loot_.items()) {
        const float distance = engine::length(item.at - feet);
        if (distance > kReleaseRadius) item.full_notice = false;  // 멀어졌으니 다음에 다시 알려요
        if (item.age < kSettleSeconds || distance > kPickupRadius) continue;
        if (bag.try_add(catalog, item.stack.item_id, item.stack.count)) {  // 전부 들어갈 때만 바뀌어요
            picked_.push_back(item);
            taken.push_back(item.id);
            log::debug("Picked up {} x{}", item.stack.item_id, item.stack.count);
        } else if (!item.full_notice) {  // 가득 참: 바닥에 두고 한 번만 알려요
            item.full_notice = true;
            refused_.push_back(item.stack);
        }
    }
    for (const auto id : taken) loot_.remove(id);  // 주운 것은 바닥에서 지워요 (같은 드롭은 한 번만)
}

}  // namespace wind::game
