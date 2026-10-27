# Changelog

## Unreleased

### Phase 0 — environment and repository
- CMake (C++20) project with `windows-debug` / `windows-release` presets (MSVC + Ninja).
- Dependencies pinned via FetchContent: SDL3 3.4.18, ENet 1.3.18, spdlog 1.15.3, GoogleTest 1.17.0.
- `shared`: `wind::log` logging interface, CLI helpers, generated version header.
- `server`: fixed-rate tick loop (20 Hz default), ENet initialization, Ctrl+C shutdown, `--max-ticks`.
- `client`: SDL3 window and renderer that clears each frame; `--max-frames`.
- 20 CTest tests, including a server start/stop smoke test.
- Build, test, and run scripts in `scripts/`; build guide in `docs/BUILD.md`.
