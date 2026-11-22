#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 "진행 상태"예요. (명세 8.2, 8.3, 13a, 개발자와 정함)
// - 능력치 다섯 개와 룬 지갑을 가지고, 성장 규칙(곡선·비용)을 알아요.
// - 필드 화면 밖(ClientApp)에 하나만 있어서, 맵을 옮기거나 쓰러져 부활해도 그대로 남아요.
//   필드는 이것을 빌려 써요: 처음 만들 때 최대 HP·MP·스태미나와 공격 보너스를 읽고,
//   몬스터를 쓰러뜨리면 룬을 넣고, 주인공이 쓰러지면 룬 절반을 잃게 해요.
// - 16단계 저장은 이 값(능력치 + 룬)을 파일로 쓰면 돼요. 레벨과 파생값은 저장하지 않고 계산해요.
// - 13b에서 마지막 쿠션(부활 자리)도 여기에 더해요.
// 게임 규칙이라 SDL을 몰라요.

#include "wind/progression/derived_stats.hpp"
#include "wind/progression/progression_rules.hpp"
#include "wind/progression/rune_wallet.hpp"
#include "wind/progression/stats.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>

namespace wind::game {

class HeroProgress {
public:
    // rules: 성장 규칙 데이터, stats: 처음 능력치 (기본 모두 10), runes: 처음 룬
    explicit HeroProgress(progression::ProgressionRules rules, progression::Stats stats = {},
                          std::int64_t runes = 0);
    // 콘텐츠 폴더의 성장 규칙 파일(progression/stat_curves.json, levels.json)을 읽어 처음 상태로 만들어요.
    // 파일이 틀리면 이유를 로그에 남기고 nullopt예요 (게임을 시작할 수 없어요).
    [[nodiscard]] static std::optional<HeroProgress> load(const std::filesystem::path& content_root);

    [[nodiscard]] const progression::Stats& stats() const { return stats_; }
    [[nodiscard]] int level() const { return progression::level_of(stats_); }
    [[nodiscard]] std::int64_t runes() const { return wallet_.amount(); }
    // 지금 능력치의 파생값 (한 곳의 계산 함수 derive_stats를 불러요)
    [[nodiscard]] progression::DerivedStats derived() const;
    // 바꿀 능력치 target의 파생값 (레벨업 화면 미리 보기, 13c)
    [[nodiscard]] progression::DerivedStats preview(const progression::Stats& target) const;

    // 지금 능력치에서 target까지 올리는 데 드는 룬.
    // target이 1~60 밖이거나, 어느 능력치든 지금보다 낮으면 nullopt (내릴 수는 없어요).
    [[nodiscard]] std::optional<std::int64_t> cost_to(const progression::Stats& target) const;
    // target으로 올려요 (전부 성공 또는 전부 실패, 명세 8.3).
    // 비용을 낼 수 있으면 룬을 한 번에 내고 능력치를 바꿔요. 아니면 아무것도 바꾸지 않고 false예요.
    bool try_raise(const progression::Stats& target);

    // 몬스터를 쓰러뜨려 룬을 받아요. 실제로 늘어난 양(상한에서 잘림)을 돌려줘요.
    std::int64_t earn(std::int64_t runes);
    // 주인공이 쓰러졌어요: 룬 절반(소수점 버림)을 잃어요. 잃은 양을 돌려줘요. 레벨·능력치는 그대로예요.
    std::int64_t fall();

private:
    progression::ProgressionRules rules_;  // 곡선과 비용 계수
    progression::Stats stats_;             // 다섯 능력치
    progression::RuneWallet wallet_;       // 룬
};

}  // namespace wind::game
