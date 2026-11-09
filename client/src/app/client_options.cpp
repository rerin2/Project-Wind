// 이 파일은 무엇을 하나요?
// 게임을 켤 때 뒤에 붙이는 말(명령줄 인자, 예: --width 1600)을 읽어서
// 실행 설정(ClientOptions)으로 바꿔 주는 곳이에요.
// 식당 주문서를 읽고 "곱빼기, 덜 맵게"를 주방 메모로 옮겨 적는 것과 비슷해요.

#include "app/client_options.hpp"

#include "wind/cli.hpp"
#include "wind/content/content_path.hpp"

#include <limits>
#include <utility>

namespace wind::client {
namespace {

// 창 크기 허용 범위 (최소 320 ~ 최대 8K 너비)
constexpr std::uint64_t kMinWindowSize = 320;
constexpr std::uint64_t kMaxWindowSize = 7680;
// 성능 시험용 파티클 개수 상한 (공중 효과 통 크기와 같아요)
constexpr std::uint64_t kMaxFxStress = 4000;
// 성능 시험용 빛 개수 상한 (한 화면에 그리는 빛 최대 개수와 같아요)
constexpr std::uint64_t kMaxLightStress = 64;

// "잘못됐어요" 결과를 만들어요. 왜 잘못됐는지 메시지를 함께 담아요.
ClientParseResult fail(std::string message) {
    return ClientParseResult{.options = std::nullopt, .error = std::move(message)};
}

}  // namespace

// 인자를 앞에서부터 하나씩 읽어요. 대부분 "--이름 값" 두 개가 한 쌍이에요.
ClientParseResult parse_client_options(std::span<const std::string_view> args) {
    ClientOptions options;  // 기본값에서 시작해서, 적힌 것만 바꿔요

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];

        // --help는 값이 없는 유일한 옵션이라 먼저 처리해요.
        if (arg == "--help" || arg == "-h") {
            return ClientParseResult{.options = std::nullopt, .error = {}, .show_help = true};
        }

        // 나머지 옵션은 모두 "--이름 값" 모양이에요. 값이 빠졌으면 오류.
        if (i + 1 >= args.size()) {
            return fail("missing value for argument: " + std::string(arg));
        }
        const std::string_view value = args[++i];  // 다음 칸이 값이에요

        if (arg == "--width" || arg == "--height") {
            const auto parsed = cli::parse_uint(value, kMinWindowSize, kMaxWindowSize);
            if (!parsed) {
                return fail(std::string(arg) + " must be an integer between " +
                            std::to_string(kMinWindowSize) + " and " +
                            std::to_string(kMaxWindowSize));
            }
            // 이름이 --width면 너비에, --height면 높이에 저장해요.
            (arg == "--width" ? options.window_width : options.window_height) =
                static_cast<std::uint32_t>(*parsed);
        } 
        else if (arg == "--max-frames") {
            // 0장은 의미가 없으니 1장 이상만 받아요.
            const auto parsed =
                cli::parse_uint(value, 1, std::numeric_limits<std::uint64_t>::max());
            if (!parsed) {
                return fail("--max-frames must be a positive integer");
            }
            options.max_frames = *parsed;
        }
         else if (arg == "--content-dir") {
            // 게임 자료 폴더 위치를 바꿔요.
            options.content_dir = std::filesystem::path(value);
        } 
        else if (arg == "--map") {
            // 맵 위치도 자료 폴더 밖을 가리키지 못하게 문지기 검사를 해요.
            auto normalized = content::normalize_content_path(value);
            if (!normalized) {
                return fail("--map must be a relative path inside the content folder");
            }
            options.map = std::move(*normalized);
        } 
        else if (arg == "--screenshot-at") {
            const auto parsed =
                cli::parse_uint(value, 1, std::numeric_limits<std::uint64_t>::max());
            if (!parsed) {
                return fail("--screenshot-at must be a positive integer");
            }
            options.screenshot_at.push_back(*parsed);
        }
        else if (arg == "--screenshot-dir") {
            options.screenshot_dir = std::filesystem::path(value);
        }
        else if (arg == "--fx-stress") {
            // 성능 시험용: 파티클을 몇 개쯤 띄울지 (0~4000)
            const auto parsed = cli::parse_uint(value, 0, kMaxFxStress);
            if (!parsed) {
                return fail("--fx-stress must be an integer between 0 and " +
                            std::to_string(kMaxFxStress));
            }
            options.fx_stress = static_cast<std::uint32_t>(*parsed);
        }
        else if (arg == "--auto-move") {
            // 시험용: up / down / left / right 중 하나를 계속 누르고 있어요.
            const auto dir = world::parse_direction(value);
            if (!dir) {
                return fail("--auto-move must be one of up|down|left|right");
            }
            options.auto_move = *dir;
        }
        else if (arg == "--light-stress") {
            // 성능 시험용: 빛을 몇 개 더 놓을지 (0~64)
            const auto parsed = cli::parse_uint(value, 0, kMaxLightStress);
            if (!parsed) {
                return fail("--light-stress must be an integer between 0 and " +
                            std::to_string(kMaxLightStress));
            }
            options.light_stress = static_cast<std::uint32_t>(*parsed);
        }
        else if (arg == "--log-level") {
            const auto parsed = log::parse_level(value);
            if (!parsed) {
                return fail("--log-level must be one of trace|debug|info|warn|error|critical|off");
            }
            options.log_level = *parsed;
        } 
        else {
            // 모르는 인자는 못 본 척하지 않고 거절해요. (오타를 빨리 알아채려고)
            return fail("unknown argument: " + std::string(arg));
        }
    }

    return ClientParseResult{.options = options};
}

// --help를 입력하면 보여 주는 사용 설명서 (영어)
std::string_view client_usage() {
    return "Usage: wind_client [options]\n"
           "  --width <px>         window width, 320-7680 (default 1280)\n"
           "  --height <px>        window height, 320-7680 (default 720)\n"
           "  --max-frames <n>     close after n frames (for smoke tests)\n"
           "  --content-dir <dir>  content root folder (default: repository content/)\n"
           "  --map <path>         map to enter, relative to the content root\n"
           "                       (default maps/test_meadow.json)\n"
           "  --log-level <level>  trace|debug|info|warn|error|critical|off (default info)\n"
           "  --screenshot-at <n>  save a screenshot after frame n; may repeat (for checks)\n"
           "  --screenshot-dir <dir>\n"
           "                       screenshot folder (default %APPDATA%\\ProjectWIND\\screenshots)\n"
           "  --fx-stress <n>      keep about n extra particles on screen, 0-4000 (perf check)\n"
           "  --light-stress <n>   add n lights around the start, 0-64; needs a lit map (perf check)\n"
           "  --auto-move <dir>    hold up|down|left|right for the whole run (for checks)\n"
           "  --help               show this message\n";
}

}  // namespace wind::client
