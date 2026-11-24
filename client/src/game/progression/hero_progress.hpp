#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 "진행 상태"예요. (명세 8.2, 8.3, 13a, 개발자와 정함)
// - 능력치 다섯 개와 룬 지갑을 가지고, 성장 규칙(곡선·비용)을 알아요.
// - 필드 화면 밖(ClientApp)에 하나만 있어서, 맵을 옮기거나 쓰러져 부활해도 그대로 남아요.
//   필드는 이것을 빌려 써요: 처음 만들 때 최대 HP·MP·스태미나와 공격 보너스를 읽고,
//   몬스터를 쓰러뜨리면 룬을 넣고, 주인공이 쓰러지면 룬 절반을 잃게 해요.
// - 16단계 저장은 이 값(능력치 + 룬)을 파일로 쓰면 돼요. 레벨과 파생값은 저장하지 않고 계산해요.
// - 13b: 마지막으로 쉰 쿠션(부활 자리)과 찾은 쿠션 목록도 가져요 (16단계 저장도 이 둘을 써요).
// - 14e: 룬 흔적(쓰러진 자리에 남은 잃은 룬) 하나를 가져요. 쓰러지면 이전 흔적은 사라지고 새로 생겨요.
// 게임 규칙이라 SDL을 몰라요.

#include "engine/core/math.hpp"
#include "game/progression/rune_trace.hpp"

#include "wind/progression/derived_stats.hpp"
#include "wind/progression/progression_rules.hpp"
#include "wind/progression/rune_wallet.hpp"
#include "wind/progression/spell_book.hpp"
#include "wind/progression/stats.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace wind::game {

// 쿠션 하나를 가리키는 주소 (어느 맵의 어느 쿠션, 13b)
struct CushionRef {
    std::string map;  // 맵 파일 (콘텐츠 폴더 기준, 예: "maps/test_meadow.json")
    std::string id;   // 쿠션 id (게임 전체에서 하나뿐)
    friend bool operator==(const CushionRef&, const CushionRef&) = default;
};

class HeroProgress {
public:
    // rules: 성장 규칙 데이터, stats: 처음 능력치 (기본 모두 10), runes: 처음 룬
    explicit HeroProgress(progression::ProgressionRules rules, progression::Stats stats = {},
                          std::int64_t runes = 0);
    // 콘텐츠 폴더의 성장 규칙 파일(progression/stat_curves.json, levels.json)을 읽어 처음 상태로 만들어요.
    // 파일이 틀리면 이유를 로그에 남기고 nullopt예요 (게임을 시작할 수 없어요).
    // start: 처음 능력치 (보통 모두 10, 시험용 --stat, 15b). 1~60 밖이면 경고하고 모두 10으로 시작해요.
    [[nodiscard]] static std::optional<HeroProgress> load(const std::filesystem::path& content_root,
                                                          const progression::Stats& start = {});

    [[nodiscard]] const progression::Stats& stats() const { return stats_; }
    [[nodiscard]] int level() const { return progression::level_of(stats_); }
    [[nodiscard]] std::int64_t runes() const { return wallet_.amount(); }
    // 지금 능력치(+ 장비 무게, 15a)의 파생값 (한 곳의 계산 함수 derive_stats를 불러요)
    // equipped_weight: 끼고 있는 장비 무게. 무거움(heavy)을 볼 때만 넘겨요 (안 넘기면 0)
    [[nodiscard]] progression::DerivedStats derived(float equipped_weight = 0.0f) const;
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
    // 주인공이 map의 at 자리에서 쓰러졌어요: 룬 절반(소수점 버림)을 잃어요. 잃은 양을 돌려줘요. 레벨·능력치는 그대로예요.
    // 14e: 되찾지 못한 이전 흔적은 영영 사라지고, 잃은 룬이 1 이상이면 그 자리에 새 흔적이 생겨요 (명세 8.3.1).
    std::int64_t fall(std::string_view map, engine::Vec2 at);
    // 마지막으로 쓰러졌을 때 잃은 룬 ("쓰러졌다" 화면에 보여 줘요, 13c). 아직 안 쓰러졌으면 0
    [[nodiscard]] std::int64_t last_lost() const { return last_lost_; }
    // 마지막으로 쓰러졌을 때 되찾지 못하고 사라진 이전 흔적의 룬 ("쓰러졌다" 화면, 14e). 없었으면 0
    [[nodiscard]] std::int64_t last_vanished() const { return last_vanished_; }

    // 룬 흔적 (14e): 없으면 nullopt. 게임 전체에 하나뿐이에요
    [[nodiscard]] const std::optional<RuneTrace>& trace() const { return trace_; }
    // 흔적을 되찾아요: 담긴 룬 전부를 지갑에 넣고(상한에서 잘림) 흔적을 지워요.
    // 지갑에 실제로 들어간 양을 돌려줘요. 흔적이 없으면 0 (한 번만 되찾아져요)
    std::int64_t recover_trace();

    // 캣닢 쿠션에서 쉬었어요 (13b): 부활 자리를 이 쿠션으로 정하고, 찾은 쿠션 목록에 넣어요.
    // 처음 찾은 쿠션이면 true (쿠션이 피어나고 배너가 떠요).
    bool rest_at(std::string_view map, std::string_view cushion_id);
    // 마지막으로 쉰 쿠션 (부활 자리). 아직 쉰 적이 없으면 nullopt (그때는 맵 기본 자리에서 부활해요)
    [[nodiscard]] const std::optional<CushionRef>& last_cushion() const { return last_cushion_; }
    // 이 쿠션을 찾은 적이 있나요? (찾은 쿠션은 피어난 모습으로 그려요)
    [[nodiscard]] bool discovered(std::string_view cushion_id) const;
    // 찾은 쿠션 id 모두 (저장용, 16b. 이름 순서)
    [[nodiscard]] std::vector<std::string> discovered_ids() const;

    // 저장에서 되살리기 (16b): 능력치·룬·흔적·부활 쿠션·찾은 쿠션을 통째로 바꿔요.
    // 능력치가 1~60 밖, 룬이 0~상한 밖, 흔적 양이 0 이하, 부활 쿠션이 찾은 목록에 없으면 아무것도 안 바꾸고 false.
    // 마술 공책은 spells().restore로 따로 되살려요.
    bool restore(const progression::Stats& stats, std::int64_t runes, std::optional<RuneTrace> trace,
                 std::optional<CushionRef> last_cushion, const std::vector<std::string>& discovered);

    // 마술 공책: 배운 마술, 마술 칸, 고른 칸 (15b). 칸 수는 derived().spell_slots예요.
    [[nodiscard]] progression::SpellBook& spells() { return spells_; }
    [[nodiscard]] const progression::SpellBook& spells() const { return spells_; }

private:
    progression::ProgressionRules rules_;  // 곡선과 비용 계수
    progression::Stats stats_;             // 다섯 능력치
    progression::RuneWallet wallet_;       // 룬
    std::int64_t last_lost_ = 0;           // 마지막으로 쓰러질 때 잃은 룬 (13c)
    std::int64_t last_vanished_ = 0;       // 마지막으로 쓰러질 때 사라진 이전 흔적의 룬 (14e)
    std::optional<RuneTrace> trace_;       // 룬 흔적 (14e, 하나뿐)
    std::optional<CushionRef> last_cushion_;           // 부활 자리 (13b)
    std::set<std::string, std::less<>> discovered_;   // 찾은 쿠션 id (쿠션 id는 게임 전체에서 하나뿐이에요)
    progression::SpellBook spells_;                    // 배운 마술·마술 칸·고른 칸 (15b)
};

}  // namespace wind::game
