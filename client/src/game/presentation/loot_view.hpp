#pragma once

// 이 파일은 무엇을 하나요?
// 바닥 아이템과 줍기의 연출이에요. (명세 9.7, 6.7, 14b) 보여 주기만 해요 (줍기 결과는 FieldLoot가 정해요).
// - 떨어질 때: 쓰러진 자리에서 떨어질 자리로 포물선을 그리며 톡 튀고, 한 번 더 작게 튀어요 (0.6초).
// - 바닥에서: 위아래로 살짝 둥실거리고, 가끔 반짝 빛나요. 밤에는 희귀도 색으로 은은하게 빛나요.
// - 주울 때: 아이콘이 주인공 몸으로 빨려 들며 작아지고, 닿으면 반짝 가루가 튀어요.
// 튀는 높이는 바닥 아이템의 나이(게임 시간)로 정해서, 맵을 다시 들어와도 이미 떨어진 것은 바닥에 있어요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/items/ground_item.hpp"
#include "game/presentation/loot_view_setup.hpp"

#include <string_view>
#include <vector>

namespace wind::items {
class ItemCatalog;
}

namespace wind::game {

class MapLoot;

// 아이템 아이콘 그림 판 (16×16 칸, 도감의 icon 번호 순서, 생성 스크립트가 그려요)
inline constexpr std::string_view kItemIconsTexture = "items/item_icons.bmp";

class LootView {
public:
    static constexpr int kIconSize = 16;  // 아이콘 한 칸 크기 (픽셀)

    LootView(LootViewSetup setup, const items::ItemCatalog& catalog);

    // 이번 박자에 주운 아이템을 모아 둬요 (박자가 여러 번이어도 빠지지 않게)
    void collect(const std::vector<GroundItem>& picked);
    // 화면 한 장마다: 둥실·반짝 시계, 빨려 드는 아이콘, 닿으면 반짝 가루. body: 주인공 몸 가운데
    void update(float dt_seconds, engine::Vec2 body, engine::ParticleRandom& random, engine::ParticlePool& air_fx);
    // 바닥 아이템(그림자·아이콘·반짝임)과 빨려 드는 아이콘을 그리기 목록에 넣어요
    void queue(const engine::Camera2D& camera, const MapLoot& loot, engine::RenderQueue& queue) const;
    // 밤에 바닥 아이템이 내는 희귀도 색 빛
    [[nodiscard]] std::vector<engine::Light> lights(const MapLoot& loot) const;

    // 바닥 아이템의 나이로 정한 "땅에서 뜬 높이"와 "지금 그릴 땅 위치" (테스트용으로도 열어 둬요)
    [[nodiscard]] static float hop_height(float age);
    [[nodiscard]] static engine::Vec2 hop_ground(const GroundItem& item);

    [[nodiscard]] std::size_t flying() const { return flying_.size(); }

private:
    struct Flying {
        int icon = 0;           // 아이콘 칸 번호
        engine::Vec2 start;     // 빨려 들기 시작한 곳 (맵 위)
        engine::Vec2 position;  // 지금 위치
        float age = 0.0f;
    };
    // 아이템 id로 아이콘 칸 번호 (도감에 없으면 0)
    [[nodiscard]] int icon_of(const GroundItem& item) const;
    void queue_icon(const engine::Camera2D& camera, int icon, engine::Vec2 center, float scale,
                    float sort_y, engine::RenderQueue& queue) const;

    LootViewSetup setup_;
    const items::ItemCatalog& catalog_;  // 아이콘·희귀도 찾기 (앱이 가지고 있어요)
    std::vector<GroundItem> pending_;    // 지난 화면 뒤에 주운 것
    std::vector<Flying> flying_;         // 빨려 드는 중인 아이콘
    float clock_ = 0.0f;                 // 둥실·반짝 시계 (초)
};

}  // namespace wind::game
