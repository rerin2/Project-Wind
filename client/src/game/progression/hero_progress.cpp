// 이 파일은 무엇을 하나요?
// 주인공 진행 상태: 파생값 계산 부르기, 레벨업 비용·확정(전부 아니면 없음), 룬 받기·잃기.

#include "game/progression/hero_progress.hpp"

#include "wind/content/progression_loader.hpp"
#include "wind/log.hpp"

#include <utility>

namespace wind::game {

HeroProgress::HeroProgress(progression::ProgressionRules rules, progression::Stats stats, std::int64_t runes)
    : rules_(std::move(rules)), stats_(stats), wallet_(runes) {}

std::optional<HeroProgress> HeroProgress::load(const std::filesystem::path& content_root) {
    auto rules = content::load_progression_rules(content_root);
    if (!rules.ok()) {
        content::log_content_errors("progression rules", rules.errors);  // 어느 파일 어느 칸이 왜 틀렸는지
        log::critical("Cannot start without valid progression rules; see errors above");
        return std::nullopt;
    }
    return HeroProgress(std::move(*rules.value));  // 능력치 모두 10, 룬 0
}

progression::DerivedStats HeroProgress::derived() const {
    return progression::derive_stats(stats_, rules_.curves);
}

progression::DerivedStats HeroProgress::preview(const progression::Stats& target) const {
    return progression::derive_stats(target, rules_.curves);
}

std::optional<std::int64_t> HeroProgress::cost_to(const progression::Stats& target) const {
    if (!progression::stats_in_range(target)) return std::nullopt;  // 1~60 밖
    for (const progression::StatId id : progression::kAllStats) {
        if (target[id] < stats_[id]) return std::nullopt;            // 내리기는 안 돼요
    }
    // 레벨 from → to를 한 칸씩 더해요 (명세 8.3: 레벨마다 비용이 달라요)
    return progression::cost_between(level(), progression::level_of(target), rules_.levels);
}

bool HeroProgress::try_raise(const progression::Stats& target) {
    const auto cost = cost_to(target);
    if (!cost) return false;                       // 이상한 목표
    if (!wallet_.try_spend(*cost)) return false;   // 룬이 모자라면 아무것도 안 바뀌어요
    const int from = level();
    stats_ = target;                               // 룬을 낸 뒤에만 능력치를 바꿔요
    log::info("Level up {} -> {} for {} runes ({} left)", from, level(), *cost, wallet_.amount());
    return true;
}

std::int64_t HeroProgress::earn(std::int64_t runes) {
    const std::int64_t added = wallet_.add(runes);
    if (added > 0) log::info("Runes +{} (now {})", added, wallet_.amount());
    return added;
}

bool HeroProgress::rest_at(std::string_view map, std::string_view cushion_id) {
    last_cushion_ = CushionRef{std::string(map), std::string(cushion_id)};  // 이제 여기서 부활해요
    const bool first = discovered_.emplace(cushion_id).second;              // 처음 넣었으면 처음 찾은 것
    log::info("Rested at cushion '{}' in '{}'{}", cushion_id, map, first ? " (first time)" : "");
    return first;
}

bool HeroProgress::discovered(std::string_view cushion_id) const {
    return discovered_.contains(cushion_id);
}

std::int64_t HeroProgress::fall(std::string_view map, engine::Vec2 at) {
    const std::int64_t lost = wallet_.lose_half();
    last_lost_ = lost;  // "쓰러졌다" 화면이 보여 줘요 (13c)
    // 14e: 되찾지 못한 이전 흔적은 영영 사라져요 (새 흔적에 더하지 않아요)
    last_vanished_ = trace_ ? trace_->amount : 0;
    trace_.reset();
    if (lost > 0) trace_ = RuneTrace{std::string(map), at, lost};  // 잃은 룬이 쓰러진 자리에 남아요
    log::info("Hero fell and lost {} runes (now {}); trace {} at ({:.0f}, {:.0f}) in '{}', {} unrecovered runes vanished",
              lost, wallet_.amount(), lost > 0 ? "left" : "not left", at.x, at.y, map, last_vanished_);
    return lost;
}

std::int64_t HeroProgress::recover_trace() {
    if (!trace_) return 0;  // 흔적이 없어요 (이미 되찾았거나 쓰러진 적 없음)
    const std::int64_t got = wallet_.add(trace_->amount);  // 전부 (상한에서 잘려요)
    log::info("Recovered {} runes from the trace in '{}' (now {})", got, trace_->map, wallet_.amount());
    trace_.reset();  // 한 번만
    return got;
}

}  // namespace wind::game
