# Changelog

## Unreleased

### Phase 3 — rendering foundation
- The frame is drawn into a 640×360 frame target, then scaled to the window. Verified pixel-identical
  to direct drawing outside the F3 panel.
- `SpriteDraw::tint` (colour multiply + opacity, replaces `alpha`) and `SpriteDraw::blend` (`Normal`, `Add`, `Multiply`).
- Tween library: `Ease` curves (`linear`, quad, cubic, `out_back`, `out_elastic`) and `Tween`.
- Screenshots: F12 or `--screenshot-at N` (repeatable), `--screenshot-dir`; default `%APPDATA%\ProjectWIND\screenshots`;
  640×360 BMP; white flash and label afterwards.
- `SdlPlatform::user_data_dir()`.
- `docs/VISUALS.md`: 32-colour palette draft, blend modes, screenshot feedback.
- 105 CTest tests.

### Spec 2.1 — finer phase plan
- Spec section 19 now has 21 phases (0–20) with a summary table; phases 0–2 stay complete.
- Spec section 22 replaced by one common prompt plus per-phase cautions.
- Phase references updated in GAMEPLAY, VISUALS, ENGINE, ADR 0007.

### Phase 2 — cleanup and free movement
- Removed the game server, ENet, `run_server.ps1`, and server tests (single-player, ADR 0007).
- Split files to one class per file pair: `ErrorList`/`LoadResult`/`JsonReader`/`json_file`,
  `AnimationClip`/`SpriteSheet`/`Animator`, `Action`/`InputBindings`/`MovementIntent`. Tests split to match.
- Free 8-way movement (`Mover`): acceleration, deceleration, normalized diagonals, float positions.
- Feet-box tile collision (`move_and_slide`): 1 px sub-steps, wall sliding, 6 px corner assist, map edge blocks.
- Dodge roll (Space/K/X): 280 px/s for 0.2 s, 0.3 s cooldown, early invulnerability flag; 0.15 s input buffer.
- Simulation/presentation split: input → fixed 60 Hz steps → per-frame effects; drawing interpolates between steps.
- Effects: feet shadow, walking dust and dodge dust burst (`ParticlePool`, 512 cap), dodge afterimages,
  camera look-ahead, walk animation speed follows movement speed.
- Debug overlay: feet position, state, speed, particle count/drops; F4 also shows the feet box.
- `Shadows` draw layer; `Renderer::create_soft_ellipse_texture`; empty source rect draws the whole texture.
- New docs: `docs/GAMEPLAY.md` (movement rules), `docs/VISUALS.md` (effect values).
- 94 CTest tests.

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
