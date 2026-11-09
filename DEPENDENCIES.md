# Dependencies

All third-party libraries are fetched by CMake `FetchContent` at configure time and pinned to an exact Git tag in [cmake/Dependencies.cmake](cmake/Dependencies.cmake). Nothing is vendored into the repository. See [docs/DECISIONS/0001-dependency-management.md](docs/DECISIONS/0001-dependency-management.md).

Adding or upgrading a library requires updating this table and recording the reason.

| Library | Pinned tag | License | Used by | Why |
|---|---|---|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | `release-3.4.18` | zlib | client | Window, input, 2D renderer backend, audio. Platform layer only; the engine is ours. Built as a static library. |
| [spdlog](https://github.com/gabime/spdlog) | `v1.15.3` | MIT (bundles {fmt}, MIT) | shared | Backend for the `wind::log` interface. |
| [nlohmann/json](https://github.com/nlohmann/json) | `v3.12.0` | MIT | shared (client, tests) | Parses content JSON (maps, tilesets, sprite sheets) without exceptions for validation. See [0005](docs/DECISIONS/0005-content-formats.md). |
| [GoogleTest](https://github.com/google/googletest) | `v1.17.0` | BSD-3-Clause | tests only | Unit test framework; `gtest_discover_tests` integrates with CTest. See [0002](docs/DECISIONS/0002-test-framework.md). |

## Build tools (not linked)

| Tool | Version verified | Notes |
|---|---|---|
| Visual Studio 2022 Build Tools (MSVC, C++ workload) | MSVC 19.44.35229 | Compiler |
| CMake | 4.4.4 | Presets require ≥ 3.25 |
| Ninja | 1.13.2 | Generator used by the presets |
| Git | 2.56.0 | Required by FetchContent |

## Planned, not yet added

- SDL3_ttf + an open-licence Korean pixel font (phase 5)
- PNG image decoder (when real art with alpha arrives; BMP + colour key for now)

## Removed

| Library | Removed | Why |
|---|---|---|
| ENet `v1.3.18` (MIT) | 2026-11-09, phase 2 | Single-player game has no networking ([ADR 0007](docs/DECISIONS/0007-single-player-visual-focus.md)) |
