# Changelog

## Unreleased

### Direction change — single-player action RPG (spec 2.0)
- `docs/MASTER_SPEC.md` rewritten: single player, no server/network/accounts/database,
  free 8-direction movement, real-time action combat, visuals (lighting, particles,
  day/night, weather, game feel, transitions) as a first-class goal, local JSON saves.
- New phase plan (spec section 19): phases 2–8. Phases 0–1 stay complete.
- `CLAUDE.md` updated to match. ADR 0007 added; ADR 0004 superseded; ADR 0001 ENet parts superseded.
- No code changed yet. Server and ENet removal is the first task of phase 2.
- Added the class-per-file module rules (spec 2.4, CLAUDE.md).

### Phase 1 — minimal engine
- Client frame loop in spec order with a fixed 60 Hz update step (0.25 s / 5-step catch-up cap).
- 640×360 logical resolution, integer scaling, nearest-neighbour textures, pixel-snapped camera.
- Engine modules: `Camera2D` (follow, bounds clamp), `InputState` (held/pressed/released, UI capture),
  data-driven sprite animation, sorted `RenderQueue` (layer → feet y → insertion), visible-tile culling.
- `Renderer` wraps SDL3 (sprites, rects, debug text, BMP textures with magenta colour key).
- `ResourceManager`: content-root-confined paths, cache, placeholder texture on failure, stats.
- Shared map/tileset model and JSON validation with file + field-path errors (`wind/content`, `wind/world`).
- Local tile movement (`GridMover`), key bindings (arrows/WASD, F3 debug, F4 collision, Esc quit).
- Debug overlay: FPS, frame time, map/tile/facing, draw calls, culled, textures, cache hits, load failures.
- Test content generator (`tools/test_content`) and a 40×30 test map with original placeholder art.
- New dependency: nlohmann/json 3.12.0. Client options `--content-dir`, `--map`.
- 79 CTest tests.

### Phase 0 — environment and repository
- CMake (C++20) project with `windows-debug` / `windows-release` presets (MSVC + Ninja).
- Dependencies pinned via FetchContent: SDL3 3.4.18, ENet 1.3.18, spdlog 1.15.3, GoogleTest 1.17.0.
- `shared`: `wind::log` logging interface, CLI helpers, generated version header.
- `server`: fixed-rate tick loop (20 Hz default), ENet initialization, Ctrl+C shutdown, `--max-ticks`.
- `client`: SDL3 window and renderer that clears each frame; `--max-frames`.
- 20 CTest tests, including a server start/stop smoke test.
- Build, test, and run scripts in `scripts/`; build guide in `docs/BUILD.md`.
