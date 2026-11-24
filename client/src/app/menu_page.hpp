#pragma once

// 이 파일은 무엇을 하나요?
// 일시정지 메뉴의 탭 하나(페이지)를 그릴 때 넘기는 준비물 묶음과, 페이지끼리 같이 쓰는 이름·색이에요. (15e)

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

#include "wind/progression/stats.hpp"

#include <array>
#include <string_view>

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class HeroProgress;
}

namespace wind::client {

struct ItemSetup;

// 페이지 하나를 그릴 준비물
struct MenuPage {
    engine::UiPainter& painter;
    engine::RectF box;                   // 페이지가 그릴 자리 (탭 줄 아래, 키 안내 위)
    float alpha = 1.0f;                  // 나타나는 정도
    const ItemSetup& items;              // 도감·장비·무기·마술 선반
    const game::HeroProgress& progress;  // 능력치·마술 공책
    engine::TextureHandle item_icons;    // 아이템 아이콘 판
    engine::TextureHandle spell_icons;   // 마술 아이콘 판
};

// 능력치 이름 (쿠션 레벨업 화면과 메뉴 능력치 탭이 같이 써요)
inline constexpr std::array<std::string_view, progression::kStatCount> kStatNames{"생명력", "정신력", "지구력", "힘", "지능"};

inline constexpr engine::Color kMenuRowFocus{255, 233, 168, 40};   // 고른 줄 바탕 (옅은 금빛, 쿠션 메뉴와 같아요)
inline constexpr engine::Color kMenuWarn{236, 96, 96, 255};        // 모자람·무거움 (빨강)
inline constexpr engine::Color kMenuGood{142, 220, 128, 255};      // 좋은 값 (연두)
inline constexpr engine::Color kMenuCellFill{0, 0, 0, 90};         // 칸 바탕 (가방 칸과 같아요)
inline constexpr engine::Color kMenuCellBorder{90, 96, 112, 160};  // 칸 테두리 (흐린 회색)
inline constexpr float kMenuDisabledAlpha = 0.4f;                  // 흐린 항목 (가방 작은 메뉴와 같아요)

}  // namespace wind::client
