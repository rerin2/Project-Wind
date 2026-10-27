#include "app/client_app.hpp"
#include "app/client_options.hpp"

#include "wind/cli.hpp"
#include "wind/log.hpp"

#include <SDL3/SDL_main.h>

#include <iostream>
#include <span>

int main(int argc, char** argv) {
    const auto args = wind::cli::to_args(argc, argv);
    const auto parsed =
        wind::client::parse_client_options(std::span(args).subspan(args.empty() ? 0 : 1));

    if (parsed.show_help) {
        std::cout << wind::client::client_usage();
        return 0;
    }
    if (!parsed.options) {
        std::cerr << "error: " << parsed.error << "\n\n" << wind::client::client_usage();
        return 2;
    }

    wind::log::init({.component = "client", .level = parsed.options->log_level});

    wind::client::ClientApp app(*parsed.options);
    const int exit_code = app.run();

    wind::log::shutdown();
    return exit_code;
}
