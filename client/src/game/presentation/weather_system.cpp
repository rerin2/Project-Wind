// 이 파일은 무엇을 하나요?
// 날씨를 고르고, 시간을 세고, 세기를 서서히 바꾸는 곳이에요.
// 순서: 날씨 A 진행 -> 시간이 다 되면 A를 서서히 약하게(세기 1 -> 0) -> 날씨 B를 서서히 세게(0 -> 1)

#include "game/presentation/weather_system.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

// F6(디버그)으로 날씨를 바꿀 때 약해지기·세지기에 각각 걸리는 시간 (초)
constexpr float kQuickTransitionSeconds = 0.4f;

std::uint8_t mix(std::uint8_t a, std::uint8_t b, float t) {
    return static_cast<std::uint8_t>(
        std::lround(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t));
}

}  // namespace

WeatherSystem::WeatherSystem(const world::WeatherTable* table, std::uint32_t seed,
                             std::string_view start_id)
    : table_(table != nullptr && !table->states.empty() ? table : nullptr), rng_(seed) {
    if (table_ == nullptr) {
        return;
    }
    std::size_t first = pick_random();
    for (std::size_t i = 0; i < table_->states.size(); ++i) {
        if (table_->states[i].id == start_id) {
            first = i;
        }
    }
    // 처음 날씨는 이미 다 와 있는 상태로 시작해요 (게임을 켜자마자 비가 "서서히" 오면 어색하니까요).
    current_ = first;
    intensity_ = 1.0f;
    remaining_ = pick_duration(table_->states[current_]);
}

void WeatherSystem::update(float dt_seconds) {
    if (table_ == nullptr || dt_seconds <= 0.0f) {
        return;
    }
    // F6으로 바꿀 때는 빠르게, 저절로 바뀔 때는 날씨 표에 적힌 시간만큼 천천히
    const float seconds = quick_ ? kQuickTransitionSeconds : table_->transition_seconds;
    const float step = dt_seconds / std::max(seconds, 0.001f);
    if (switching_) {
        // 지금 날씨를 약하게 하는 중
        intensity_ -= step;
        if (intensity_ <= 0.0f) {
            current_ = queued_;
            intensity_ = 0.0f;
            switching_ = false;
            remaining_ = pick_duration(table_->states[current_]);
        }
        return;
    }
    // 새 날씨를 세게 하는 중이면 1까지 올려요. 다 세지면 "빠르게" 표시를 지워요.
    intensity_ = std::min(1.0f, intensity_ + step);
    if (intensity_ >= 1.0f) {
        quick_ = false;
    }
    remaining_ -= dt_seconds;
    if (remaining_ <= 0.0f) {
        const std::size_t next_index = pick_random();
        if (next_index == current_) {
            remaining_ = pick_duration(table_->states[current_]);  // 같은 날씨면 그냥 더 이어가요
        } else {
            start(next_index);
        }
    }
}

void WeatherSystem::next() {
    if (table_ == nullptr) {
        return;
    }
    // 바꾸는 중이면 그 다음 것으로, 아니면 지금의 다음 것으로
    const std::size_t from = switching_ ? queued_ : current_;
    start((from + 1) % table_->states.size());
    quick_ = true;  // 디버그 키는 결과가 바로 보여야 해요
}

const world::WeatherState* WeatherSystem::upcoming() const {
    return table_ != nullptr && switching_ ? &table_->states[queued_] : nullptr;
}

void WeatherSystem::start(std::size_t index) {
    queued_ = index;
    switching_ = true;  // 지금 날씨부터 서서히 약하게
}

const world::WeatherState* WeatherSystem::current() const {
    return table_ != nullptr ? &table_->states[current_] : nullptr;
}

world::Rgba WeatherSystem::tint() const {
    const auto* state = current();
    if (state == nullptr) {
        return {255, 255, 255, 255};
    }
    return {mix(255, state->tint.r, intensity_), mix(255, state->tint.g, intensity_),
            mix(255, state->tint.b, intensity_), 255};
}

float WeatherSystem::fog() const {
    const auto* state = current();
    return state != nullptr ? state->fog * intensity_ : 0.0f;
}

std::size_t WeatherSystem::pick_random() {
    float total = 0.0f;
    for (const auto& state : table_->states) {
        total += state.weight;
    }
    std::uniform_real_distribution<float> dist(0.0f, total);
    float roll = dist(rng_);
    for (std::size_t i = 0; i < table_->states.size(); ++i) {
        roll -= table_->states[i].weight;
        if (roll <= 0.0f && table_->states[i].weight > 0.0f) {
            return i;
        }
    }
    return table_->states.size() - 1;
}

float WeatherSystem::pick_duration(const world::WeatherState& state) {
    std::uniform_real_distribution<float> dist(state.min_seconds, std::max(state.min_seconds, state.max_seconds));
    return dist(rng_);
}

}  // namespace wind::game
