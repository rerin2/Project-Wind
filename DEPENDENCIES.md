# Dependencies

All third-party libraries are fetched by CMake `FetchContent` at configure time and pinned to an exact Git tag in [cmake/Dependencies.cmake](cmake/Dependencies.cmake). No library code is vendored into the repository (the only third-party files in the repository are the font assets listed under "Bundled content assets"). See [docs/DECISIONS/0001-dependency-management.md](docs/DECISIONS/0001-dependency-management.md).

Adding or upgrading a library requires updating this table and recording the reason.

| Library | Pinned tag | License | Used by | Why |
|---|---|---|---|---|
| [SDL3](https://github.com/libsdl-org/SDL) | `release-3.4.18` | zlib | client | Window, input, 2D renderer backend, audio. Platform layer only; the engine is ours. Built as a static library. |
| [spdlog](https://github.com/gabime/spdlog) | `v1.15.3` | MIT (bundles {fmt}, MIT) | shared | Backend for the `wind::log` interface. |
| [nlohmann/json](https://github.com/nlohmann/json) | `v3.12.0` | MIT | shared (client, tests) | Parses content JSON (maps, tilesets, sprite sheets) without exceptions for validation. See [0005](docs/DECISIONS/0005-content-formats.md). |
| [GoogleTest](https://github.com/google/googletest) | `v1.17.0` | BSD-3-Clause | tests only | Unit test framework; `gtest_discover_tests` integrates with CTest. See [0002](docs/DECISIONS/0002-test-framework.md). |
| [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf) | `release-3.2.2` | zlib | client (`text_renderer.cpp` only) | Draws TrueType text for Korean UI (phase 8, spec 6.9). Static. Built with only its vendored **FreeType 2.13.2** (SDL fork, submodule `external/freetype`, used under the FreeType License (FTL), its other option being GPLv2). HarfBuzz and plutosvg are off: Hangul needs no shaping and we draw no colour emoji. |

### Bundled content assets

| Asset | Version | License | Where | Why |
|---|---|---|---|---|
| [Galmuri](https://github.com/quiple/galmuri) — `Galmuri11.ttf`, `Galmuri11-Bold.ttf` | `v2.40.4` (from `dist/`) | SIL Open Font License 1.1 (licence text shipped as `content/fonts/Galmuri-OFL.md`) | `content/fonts/` | Korean pixel font designed for 12 px, fits the 640×360 screen (chosen over NeoDunggeunmo 16 px on 2026-11-10). OFL allows bundling with the game; the font may not be sold on its own and its Reserved Font Name stays with the author. |

### Credits required by licences

- FreeType (FTL, via SDL3_ttf): "Portions of this software are copyright © 2023 The FreeType Project (www.freetype.org). All rights reserved." This line must appear in the game's credits or documentation when the game is distributed (phase 20).
- Galmuri: copyright © 2019–2025 Lee Minseo, SIL OFL 1.1. Ship `Galmuri-OFL.md` next to the font files.

## Build tools (not linked)

| Tool | Version verified | Notes |
|---|---|---|
| Visual Studio 2022 Build Tools (MSVC, C++ workload) | MSVC 19.44.35229 | Compiler |
| CMake | 4.4.4 | Presets require ≥ 3.25 |
| Ninja | 1.13.2 | Generator used by the presets |
| Git | 2.56.0 | Required by FetchContent |

## Planned, not yet added

- PNG image decoder (when real art with alpha arrives; BMP + colour key for now)

## Removed

| Library | Removed | Why |
|---|---|---|
| ENet `v1.3.18` (MIT) | 2026-11-09, phase 2 | Single-player game has no networking ([ADR 0007](docs/DECISIONS/0007-single-player-visual-focus.md)) |
