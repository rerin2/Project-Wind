// 이 파일은 무엇을 하나요?
// 개발자용 정보판의 숫자를 계산하고, 글자를 화면에 실제로 그리는 곳이에요.

#include "app/debug_overlay.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <string>

namespace wind::client {
namespace {

constexpr double kFpsWindowSeconds = 0.5;  // FPS를 몇 초마다 새로 계산할지
constexpr float kMargin = 4.0f;            // 정보판 가장자리 여백 (픽셀)
constexpr float kLineHeight = engine::Renderer::kDebugCharSize + 2.0f;  // 줄 간격: 글자 높이 + 2픽셀
constexpr engine::Color kTextColor{235, 235, 235, 255};  // 글자색: 밝은 회색
constexpr engine::Color kPanelColor{0, 0, 0, 160};       // 판 색: 반투명 검정

}  // namespace

void DebugOverlay::record_frame(double frame_seconds) {
    window_seconds_ += frame_seconds;
    ++window_frames_;
    // 0.5초가 모이면: FPS = 그린 장수 ÷ 걸린 시간, 그리고 다시 0부터 모아요.
    if (window_seconds_ >= kFpsWindowSeconds) {
        fps_ = static_cast<double>(window_frames_) / window_seconds_;
        frame_ms_ = window_seconds_ * 1000.0 / static_cast<double>(window_frames_);
        window_seconds_ = 0.0;
        window_frames_ = 0;
    }
}

void DebugOverlay::draw(engine::Renderer& renderer, const game::FieldDebugInfo& field,
                        const engine::ResourceStats& resources,
                        std::uint64_t dropped_frames) const {
    if (!visible_) {
        return;  // 숨김 상태
    }
    const auto& stats = renderer.last_frame_stats();  // 지난 화면의 그리기 통계
    // 정보판 글꼴은 영어·숫자(ASCII)만 쓸 수 있어서, 한국어 맵 이름 대신 영문 map_id를 보여 줘요.
    // 한 줄씩 보여 줄 글자들:
    const std::array lines{
        std::format("FPS {:.1f}  frame {:.2f} ms  dropped {}", fps_, frame_ms_, dropped_frames),
        std::format("map {}  pos ({:.1f}, {:.1f})  tile ({}, {})", field.map_id,
                    field.player_feet.x, field.player_feet.y, field.player_tile.x,
                    field.player_tile.y),
        std::format("{}  facing {}  speed {:.0f} px/s", field.state,
                    world::to_string(field.facing), field.speed),
        std::format("draw calls {}  culled {}  tiles {}", stats.draw_calls, stats.culled,
                    field.tiles_queued),
        std::format("particles {}  dropped {}  ambient fx {}", field.particles,
                    field.particles_dropped, field.ambient_effects),
        field.lighting
            ? std::format("lights {}  drawn {}  offscreen {}  over cap {}", field.lights.total,
                          field.lights.drawn, field.lights.offscreen, field.lights.over_cap)
            : std::string("lighting off"),
        std::format("textures {}  cache hits {}  load failures {}", resources.textures_loaded,
                    resources.cache_hits, resources.load_failures),
        std::string("arrows/WASD move  Space dodge  F3 debug  F4 collision  F12 shot  Esc quit"),
    };

    // 글자 뒤에 반투명 검은 판을 깔아서, 풀밭 위에서도 글자가 잘 읽히게 해요.
    // 판의 크기는 가장 긴 줄에 맞춰요.
    std::size_t longest = 0;
    for (const auto& line : lines) {
        longest = std::max(longest, line.size());
    }
    const float width = static_cast<float>(longest) * engine::Renderer::kDebugCharSize + kMargin * 2.0f;
    const float height = static_cast<float>(lines.size()) * kLineHeight + kMargin * 2.0f;
    renderer.fill_rect({0.0f, 0.0f, width, height}, kPanelColor);

    // 위에서부터 한 줄씩 아래로 내려가며 글자를 써요.
    float y = kMargin;
    for (const auto& line : lines) {
        renderer.draw_debug_text({kMargin, y}, line, kTextColor);
        y += kLineHeight;
    }
}

}  // namespace wind::client
