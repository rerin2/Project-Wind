# Dependencies

All third-party libraries are fetched by CMake `FetchContent` at configure time and pinned to an exact Git tag in [cmake/Dependencies.cmake](cmake/Dependencies.cmake). Nothing is vendored into the repository. See [docs/DECISIONS/0001-dependency-management.md](docs/DECISIONS/0001-dependency-management.md).

Adding or upgrading a library requires updating this table and recording the reason.

| Library | Pinned tag | License | Used by | Why |
|---|---|---|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | `release-3.4.18` | zlib | client | Window, input, 2D renderer backend, audio. Platform layer only; the engine is ours. Built as a static library. |
| [ENet](https://github.com/lsalzman/enet) | `v1.3.18` | MIT | server, client (from phase 2) | Reliable/unreliable UDP transport. The application protocol is ours (`docs/PROTOCOL.md`, phase 2). |
| [spdlog](https://github.com/gabime/spdlog) | `v1.15.3` | MIT (bundles {fmt}, MIT) | shared | Backend for the `wind::log` interface. |
| [GoogleTest](https://github.com/google/googletest) | `v1.17.0` | BSD-3-Clause | tests only | Unit test framework; `gtest_discover_tests` integrates with CTest. See [0002](docs/DECISIONS/0002-test-framework.md). |

## Build tools (not linked)

| Tool | Version verified | Notes |
|---|---|---|
| Visual Studio 2022 Build Tools (MSVC, C++ workload) | MSVC 19.44.35229 | Compiler |
| CMake | 4.4.4 | Presets require ≥ 3.25 |
| Ninja | 1.13.2 | Generator used by the presets |
| Git | 2.56.0 | Required by FetchContent |

## Planned, not yet added

- PostgreSQL client library (phase 4)
- Password hashing (Argon2id implementation, phase 4) — to be chosen and recorded before use
- JSON parser for content data (phase 1/3)
