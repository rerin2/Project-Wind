#pragma once

// 이 파일은 무엇을 하나요?
// 게임을 켤 때 "새 게임인가, 저장을 불러오나"를 정하고 준비해요. (명세 12.2, 16b)
// 순서: --new-game이면 새 게임 → 아니면 SavePicker로 본/자동 저장 확인 →
//       손상·미래 버전이면 시작 질문 창 → 불러올 저장이면 콘텐츠 확인이 끝난 내용을 게임 상태에 되살림.
// 시험용 시작 옵션은 새 게임에서만 쓰여요(이미 ItemSetup·HeroProgress에 들어 있고, 불러오면 덮여요).

#include "engine/core/math.hpp"
#include "game/entities/hero_vitals.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace wind::engine {
class SdlPlatform;
class UiPainter;
}  // namespace wind::engine

namespace wind::game {
class InputBindings;
}

namespace wind::client {

struct ClientOptions;
struct ItemSetup;
class SaveKeeper;

// 어디서 시작할지
struct StartPoint {
    std::string map;                          // 첫 맵
    std::optional<engine::Vec2> feet;         // 불러온 발 위치 (새 게임이면 없음 → 맵 기본 자리)
    std::optional<game::HeroVitals> vitals;   // 불러온 HP·MP (새 게임이면 없음 → 가득)
};

// 저장 폴더: --save-dir, 없으면 %APPDATA%\ProjectWIND\saves (자료 폴더도 없으면 실행 폴더의 saves)
[[nodiscard]] std::filesystem::path save_folder(const ClientOptions& options);

// 시작할 곳을 정해요. nullopt면 게임을 꺼요(미래 버전 저장, 창 닫기, 파일 정리 실패).
// frame: 질문 창이 그린 장수를 이어 세요
[[nodiscard]] std::optional<StartPoint> choose_start(const ClientOptions& options, SaveKeeper& saver, ItemSetup& items,
                                                     engine::SdlPlatform& platform, engine::UiPainter& painter,
                                                     const game::InputBindings& bindings, std::uint64_t& frame);

}  // namespace wind::client
