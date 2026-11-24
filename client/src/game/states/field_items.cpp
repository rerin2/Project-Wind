// 이 파일은 무엇을 하나요?
// 필드 아이템 담당: 규칙(FieldLoot)과 연출(LootView)을 차례로 부르고, 알림을 모아요.

#include "game/states/field_items.hpp"

#include "game/entities/player_motion.hpp"
#include "game/items/drop_roll.hpp"
#include "game/monsters/monster.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"
#include "wind/items/quick_slots.hpp"
#include "wind/log.hpp"

namespace wind::game {
namespace {

constexpr float kBodyLift = 10.0f;  // 발에서 몸 가운데까지 (아이템이 빨려 드는 곳)
static_assert(kQuickSlotActions.size() == items::QuickSlots::kCount);  // 키 수 = 단축 칸 수

}  // namespace

void FieldItems::handle_quick_keys(const engine::InputState& input, const InputBindings& bindings) {
    for (std::size_t i = 0; i < kQuickSlotActions.size(); ++i) {
        if (bindings.pressed(kQuickSlotActions[i], input)) requests_.push_back({Request::Kind::Quick, i});
    }
}

void FieldItems::use_quick(std::size_t quick_slot, PlayerMotion& hero) {
    if (items_.quick == nullptr) return;                                     // 단축 칸이 없는 필드 (테스트 등)
    const auto bag_slot = items_.quick->stack_for(quick_slot, items_.bag);  // 가방의 첫 묶음
    if (!bag_slot) return;                                                   // 비었거나 다 썼어요 (HUD가 흐리게 보여 줘요)
    use(*bag_slot, hero);                                                    // 물약 쓰기와 같은 규칙 (못 마시면 알림)
}

FieldItems::FieldItems(const ItemWorld& items, MapLoot& loot, const LootViewSetup& view, std::uint64_t seed)
    : items_(items), loot_(loot, seed), view_(view, items.catalog), drink_view_(view) {}

bool FieldItems::drops_known(const std::vector<Monster>& monsters, const items::ItemCatalog& catalog) {
    for (const auto& monster : monsters) {
        if (const auto bad = unknown_drop_item(monster.data().drops, catalog)) {
            log::error("Monster {} drops unknown item {}", monster.data().id, *bad);
            return false;
        }
    }
    return true;
}

void FieldItems::step(float dt_seconds, const FieldCombat& combat, PlayerMotion& hero, const world::MapData& map) {
    for (const auto& request : requests_) {  // 인벤토리 화면 요청부터 (14c·14d)
        if (request.kind == Request::Kind::PutDown) (void)loot_.put_down(request.slot, hero, map, items_.bag);
        if (request.kind == Request::Kind::Sort) (void)items_.bag.sort(items_.catalog);  // 칸이 모자랄 일은 없어요
        if (request.kind == Request::Kind::Use) use(request.slot, hero);
        if (request.kind == Request::Kind::Quick) use_quick(request.slot, hero);  // 1~4 (15f)
    }
    requests_.clear();
    loot_.step(dt_seconds, combat, hero, map, items_.bag, items_.catalog);
    view_.collect(loot_.picked());  // 주운 것은 빨려 드는 연출로
    for (const auto& item : loot_.picked()) notices_.push_back({item.stack, LootNotice::Kind::PickedUp});
    for (const auto& stack : loot_.refused()) notices_.push_back({stack, LootNotice::Kind::BagFull});
}

// 물약 쓰기 (14d): 먼저 다 확인하고, 될 때만 가방에서 하나 꺼내 마시기 시작해요 (전부 성공 또는 전부 실패)
void FieldItems::use(std::size_t slot, PlayerMotion& hero) {
    const auto& stack = items_.bag.at(slot);
    if (!stack) return;  // 그사이 칸이 비었어요
    const auto* def = items_.catalog.find(stack->item_id);
    if (def == nullptr || !def->use) return;  // 쓸 수 없는 아이템 (화면이 이미 막아요)
    const items::ItemStack one{stack->item_id, 1};
    if (!hero.can_drink()) {  // 공격·구르기·맞는 중 등
        notices_.push_back({one, LootNotice::Kind::CannotUse});
        return;
    }
    if (!hero.drink_would_help(*def->use)) {  // HP(또는 MP)가 이미 가득
        notices_.push_back({one, LootNotice::Kind::AlreadyFull});
        return;
    }
    const auto effect = *def->use;
    if (!items_.bag.take_at(slot, 1)) return;  // 위에서 확인했으니 실패하지 않아요
    (void)hero.begin_drink(effect);             // can_drink를 확인했으니 시작돼요
    drink_view_.start(def->icon, effect);       // 손에 든 병 그림과 꿀꺽 반짝임
    log::debug("Drinking {} (hp +{}, mp +{})", def->id, effect.heal_hp, effect.heal_mp);
}

void FieldItems::update(float dt_seconds, const PlayerMotion& hero, engine::Vec2 feet, engine::ParticleRandom& random,
                        engine::ParticlePool& air_fx) {
    view_.update(dt_seconds, feet - engine::Vec2{0.0f, kBodyLift}, random, air_fx);
    drink_view_.update(dt_seconds, hero, feet, random, air_fx);  // 물약 병 기울이기, 꿀꺽 반짝임 (14d)
}

void FieldItems::queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const {
    view_.queue(camera, loot_.loot(), queue);
    drink_view_.queue(camera, queue);
}

std::vector<engine::Light> FieldItems::lights() const {
    auto out = view_.lights(loot_.loot());
    if (const auto light = drink_view_.light()) out.push_back(*light);  // 꿀꺽할 때 잠깐 빛나요
    return out;
}

}  // namespace wind::game
