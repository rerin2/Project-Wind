#pragma once

// 이 파일은 무엇을 하나요?
// 게임을 켤 때 고를 수 있는 설정(창 크기, 맵 파일 등)을 담는 "주문서"의 모양을 정해요.
// SDL(창 도구 상자)을 쓰지 않아서, 창을 띄우지 않고도 숙제 검사(테스트)를 할 수 있어요.

#include "wind/log.hpp"
#include "wind/world/direction.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace wind::client {

// 실행 설정. 아무것도 안 적어도 기본값으로 잘 실행돼요.
struct ClientOptions {
    std::uint32_t window_width = 1280;   // 창 너비(픽셀)
    std::uint32_t window_height = 720;   // 창 높이(픽셀)
    // 이 장수만큼 그리면 창을 저절로 닫아요. (자동 시험용) 비어 있으면 사용자가 닫을 때까지 실행.
    std::optional<std::uint64_t> max_frames;
    log::Level log_level = log::Level::Info;  // 기록(로그)을 얼마나 자세히 남길지
    // 게임 자료(콘텐츠) 폴더. 기본값은 빌드할 때의 저장소 content 폴더예요.
    std::filesystem::path content_dir = WIND_DEFAULT_CONTENT_DIR;
    // 처음 들어갈 맵 (콘텐츠 폴더 기준 위치)
    std::string map = "maps/test_meadow.json";
    // 이 장(프레임) 번호들을 그린 뒤 스크린샷을 저장해요. (자동 시험용, 키를 누르지 않아도 돼요)
    // --screenshot-at을 여러 번 쓰면 여러 장을 찍어요.
    std::vector<std::uint64_t> screenshot_at;
    // 성능 시험: 화면에 약 이만큼의 파티클을 계속 띄워요 (0이면 끔, 최대 4000).
    std::uint32_t fx_stress = 0;
    // 성능 시험: 시작 위치 주변에 빛을 이만큼 더 놓아요 (0이면 끔, 최대 64).
    std::uint32_t light_stress = 0;
    // 시험용: 이 방향키를 처음부터 끝까지 누르고 있는 것처럼 해요 (화면 확인 자동화).
    std::optional<world::Direction> auto_move;
    // 시험용: 정해진 장(프레임)에서 키를 한 번 눌렀다 떼요. 예: --press f6@60 (여러 번 쓸 수 있어요)
    struct KeyPress {
        std::string key;       // 키 이름: f3 f4 f5 f6 f12 space up down left right
        std::uint64_t frame = 0;
    };
    std::vector<KeyPress> presses;
    // 시작 시각 (0~24). 비어 있으면 아침 9시.
    std::optional<float> start_hour;
    // 처음 날씨 이름 ("rain" 등). 비어 있으면 날씨 표의 확률대로.
    std::string start_weather;
    // 스크린샷 저장 폴더. 비어 있으면 %APPDATA%\ProjectWIND\screenshots
    std::filesystem::path screenshot_dir;
};

// 주문서를 읽은 결과. 셋 중 하나예요: 성공 / 실패 / 도움말 보여 주기
struct ClientParseResult {
    std::optional<ClientOptions> options;  // 성공하면 여기에 설정이 들어 있어요
    std::string error;                     // 실패하면 사람이 읽을 이유가 들어 있어요
    bool show_help = false;                // --help를 입력했으면 true
};

// 명령줄 인자(켤 때 뒤에 붙인 말)를 읽어요. 프로그램 이름(argv[0])은 빼고 넘겨 줘야 해요.
[[nodiscard]] ClientParseResult parse_client_options(std::span<const std::string_view> args);

// --help나 오류 때 보여 줄 사용 설명서
[[nodiscard]] std::string_view client_usage();

}  // namespace wind::client
