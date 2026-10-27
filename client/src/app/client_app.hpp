#pragma once

#include "app/client_options.hpp"

#include <cstdint>

namespace wind::client {

// Top-level client loop. Phase 0 only opens a window and clears it each frame;
// the full frame order from master spec 5.1 is filled in during phase 1.
class ClientApp {
public:
    explicit ClientApp(ClientOptions options);

    // Returns a process exit code: 0 on clean shutdown.
    int run();

private:
    ClientOptions options_;
    std::uint64_t frames_rendered_ = 0;
};

}  // namespace wind::client
