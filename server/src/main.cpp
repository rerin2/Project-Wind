#include "core/server_app.hpp"
#include "core/server_options.hpp"

#include "wind/cli.hpp"
#include "wind/log.hpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <span>

namespace {

// Set from the signal handler; lock-free atomics are signal-safe.
std::atomic<bool> g_stop_requested{false};
static_assert(std::atomic<bool>::is_always_lock_free);

extern "C" void handle_stop_signal(int /*signal*/) {
    g_stop_requested.store(true);
}

void install_signal_handlers() {
    std::signal(SIGINT, handle_stop_signal);
    std::signal(SIGTERM, handle_stop_signal);
#ifdef SIGBREAK
    std::signal(SIGBREAK, handle_stop_signal);
#endif
}

}  // namespace

int main(int argc, char** argv) {
    const auto args = wind::cli::to_args(argc, argv);
    const auto parsed =
        wind::server::parse_server_options(std::span(args).subspan(args.empty() ? 0 : 1));

    if (parsed.show_help) {
        std::cout << wind::server::server_usage();
        return 0;
    }
    if (!parsed.options) {
        std::cerr << "error: " << parsed.error << "\n\n" << wind::server::server_usage();
        return 2;
    }

    wind::log::init({.component = "server", .level = parsed.options->log_level});
    install_signal_handlers();

    wind::server::ServerApp app(*parsed.options);
    const int exit_code = app.run(g_stop_requested);

    wind::log::shutdown();
    return exit_code;
}
