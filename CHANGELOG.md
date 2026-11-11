# Changelog

## Unreleased

### Phase 9a — attack and weapon data, damage
- `shared/combat`: `AttackData` (windup / active / recovery, cancel window, hitbox, power, knockback, hitstop, shake,
  optional mp_cost, fx, sfx, next_combo), `Weapon`, pure `compute_damage` and `physical_attack_power` (spec 9.3, 8.2).
- Validated loaders `attack_loader` and `weapon_loader`; a weapon loads its combo attacks and checks their order.
- Longsword data: `content/weapons/longsword.json` and three attacks (`content/attacks/longsword_1..3.json`).
- 261 CTest tests.

### Phase 8 — text, UI and stamina
- SDL3_ttf `release-3.2.2` (static, vendored FreeType 2.13.2 only) and the Galmuri11 Korean pixel font
  (v2.40.4, OFL 1.1, regular + bold) at 12 px with mono hinting. Licences and the FreeType credit line in DEPENDENCIES.md.
- `TextRenderer` (SDL_ttf, cached text), SDL-free `utf8` helpers and `layout_text`: wrap at spaces, break long words
  by glyph, align, clip with `...`; clipped UI text is counted on F3 and logged once.
- UI: theme file `content/ui/theme.json` with loader, generated 9-slice frames (`content/ui/ui_frames.bmp`),
  `nine_slice`, `FocusList`, `UiPainter`; F7 UI test panel (buttons with focus, blocks game input, Esc closes it).
- Area name banner on map entry; maps now have Korean display names (시험 초원 / 시험 동굴 / 시험 숲).
  The "cannot enter" travel notice is now Korean.
- Stamina (spec 7.2.1): `Stamina`, `PlayerMotion`; Shift run at 165 px/s (25/s), dodge costs 25, 0.6 s regen delay,
  30/s regen, exhaustion until 25. HUD stamina bar with delayed trail, auto-dim when full, red blink when exhausted.
- `PlayerSprite` split out of `field_state.cpp` (364 → 316 lines); `ui_resources` and `transition_view` split out of
  `client_app.cpp`. F3 panel now starts hidden (HUD is in that corner) and shows stamina and UI text stats;
  its help is two lines so it fits 640 px. Key names table completed.
- Test option `--hold key@from-to`; `--press` accepts f7, shift, enter, esc.
- 240 CTest tests.

### Phase 7 — living world and region travel
- Tileset `animation` (frames + seconds per frame) and `sway` (pixels + period). Water shimmers (4 → 14 → 15 → 14),
  flowers lean (9 ↔ 16), tree canopies and tall grass sway 1 px in a wave of wind. Neighbouring cells are staggered.
- Exit `transition`: `fade` (default) or `iris` (a pixel-exact circle closing on the hero, opening on her in the new map).
  Meadow ↔ cave uses iris.
- New test forest (36×24): forest floor, tall grass, mushrooms, dark trees, a shimmering stream, bush border,
  falling leaves, foggy weather table. Reached from the right end of the meadow path (fade).
- `MapTravel` (SDL-free) runs cover → enter → uncover. A bad exit target keeps the current map, logs the reason and
  shows a red notice for 4 s. Tests cover the bad target and follow every shipped exit.
- Tileset parsing moved to `tileset_loader` (map_loader.cpp 360 → 307 lines); `JsonReader::schema_version`.
  `load_field_assets` moved into `wind_game_core` so it can be tested. Test key helpers moved to `scripted_input`.
- 202 CTest tests.

### Fix: F6 seemed to do nothing
- F6 used the table's 5 s fade out + 5 s fade in, so from clear nothing visible happened for 5 s.
  F6 now fades 0.4 s each way; natural weather changes keep the table's slower fade.
- F3 shows a change in progress (`clear -> rain`); the log records each F6 change (or says the map has no weather).
- Test option `--press <key>@<frame>` taps a key at a frame. Verified clear → rain → snow on screen.
- 182 CTest tests.

### Phase 6 — time and weather
- Game clock (60 real s = 1 game hour, starts 09:00) owned by the game loop, so time continues across maps. F5 skips 3 hours.
- Day cycle files (`content/time/`): colour + night keys blended over the day, wrapping past midnight.
- Weather tables (`content/weather/`): weighted states with durations, effects, tint and fog; 5 s fades between states.
  F6 next weather. The meadow has clear, rain (streaks + splashes), snow and fog.
- `FieldAtmosphere`: outdoor ambient = time colour × weather tint; night effects (fireflies) scale with night.
  Indoor maps with a fixed `ambient_light` ignore time and weather.
- Lanterns (tile 13) with `night_only` lights along the meadow path.
- New particle shape `streak`; emitter `rate_scale`; `Glow` draw layer after lighting for additive particles.
- Shared `JsonReader::range`, `color`, `content_path` replace copies in three loaders. Effect `rate` cap 500 → 2000.
- `--time`, `--weather` options. Field loading moved to `field_assets`.
- 181 CTest tests.

### Way into the test cave (early part of phase 7)
- Map `exits` (area in tiles, `to_map`, `to_spawn`) with validation, including "at least one walkable tile".
- Walking onto an exit fades to black, loads the target map at the named spawn, and fades back in
  (`ScreenFade`, 0.35 s each way). If the target cannot load, the hero stays and the error is logged.
- Exits arm only after stepping off exit tiles, so arriving next to one never bounces back.
- Meadow: cave mouth (new tile 12) at the left end of the dirt path; cave: arch exit back on its left wall.
- `FieldState::create` takes a spawn id; each field frees its generated textures when replaced.
- Test option `--auto-move <dir>`; verified meadow → cave → meadow with screenshots.
- 153 CTest tests.

### Phase 5 — lighting
- Light map pass on the SDL renderer: ambient colour, lights added (`ADD`), then multiplied over the frame (`MOD`).
  Foreground and debug layers draw after it. Decision: ADR 0008.
- Engine `light` module: `Light`, flicker and pulse animation, `queue_lights` (off-screen cull, 64 per frame cap).
- Map `ambience` gains `ambient_light`, `player_light` and `lights` (with optional `flicker`, `pulse`, `fx`);
  parsed by the new `ambience_loader`. Colour parsing moved to shared `hex_color`.
- `FieldLighting`: map lights, a light that follows the hero, ember sparks at torches.
- Content: torch tile (11), `fx/torch_embers.json`, test cave map `maps/test_cave.json`.
- F3 shows light counts; `--light-stress N`: 42 lights (37 drawn) at 60.0 FPS.
- Refactors for the 300-line rule: `surface_painter` (procedural textures), `field_effects` (effect loading),
  shared `log_content_errors`.
- 147 CTest tests.

### Phase 4 — particle system
- Particle effects are data: `content/fx/*.json` with validation (file + field path errors). Fields: shape, blend,
  lifetime/speed/size ranges, direction, spread, gravity, drag, spawn radius, colour gradient, fade in/out,
  wobble, burst, rate.
- Engine: `ParticleEffect` + loader, `ParticleRandom`, `make_particle`/`emit_burst`, continuous `ParticleEmitter`,
  colour/fade/wobble drawing with `soft` and `pixel` shapes and per-particle blend mode.
- Dust now uses `fx/dust_walk.json` and `fx/dust_dodge.json`. Broken effect files are logged and skipped.
- Maps can list `ambience.ambient_fx`; the test meadow has drifting petals and fireflies (`WorldFx` layer, above canopies).
- `JsonReader::number` (decimals) and `JsonReader::raw`; `Renderer::create_solid_texture`.
- `--fx-stress N`: 2039 particles measured at 60.0 FPS (VSync), 1654 draw calls, 0 dropped.
- 131 CTest tests.

### Hero redesign
- The hero test sprite is now a silver-haired girl with cat ears and long straight hair: navy dress, violet eyes,
  1 px dark outline, hair tips sway while walking. Right-facing frames mirror the left-facing ones.
- Only `content/sprites/test_hero.bmp` changed; tiles and the test map regenerate byte-identical.
- F3 panel help line mentions F12.

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
