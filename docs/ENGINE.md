# Engine Design (client)

Status: phase 6 (time of day, weather, fog, night-only lights, glow layer). Spec reference: master spec section 5.

The client engine is our own code on top of SDL3. SDL3 is used only for the window, OS events, the GPU-backed 2D renderer, and BMP decoding. Everything else (loop, map, camera, animation, input model, particles, resources, debug view) lives in this repository.

## Module layout

| Folder | Library | SDL? | Responsibility |
|---|---|---|---|
| `client/src/engine/core` | `wind_engine_core` | no | `Vec2`/`RectF` math, `FixedStepClock` |
| `client/src/engine/map` | `wind_engine_core` | no | tile↔world coordinates, visible-tile culling, tile batching into the render queue, animated and swaying tiles (`tile_animation`) |
| `client/src/engine/camera` | `wind_engine_core` | no | `Camera2D`: world↔screen, follow, bounds clamp, pixel snapping; `LookAhead`: smoothed offset toward the movement direction |
| `client/src/engine/input` | `wind_engine_core` | no | `InputState`: held/pressed/released, UI capture flag |
| `client/src/engine/animation` | `wind_engine_core` | no | `AnimationClip`, `SpriteSheet`, `Animator` (one file each), sprite sheet JSON loader |
| `client/src/engine/fx` | `wind_engine_core` | no | `Particle`, `ParticleEffect` + loader, `ParticlePool` (fixed capacity, drop counter), `ParticleRandom`, `make_particle`/`emit_burst`, `ParticleEmitter` (continuous), `queue_particles`, `ScreenFade` (fade / iris progress), `iris_mask` |
| `client/src/engine/light` | `wind_engine_core` | no | `Light`, `light_strength`/`light_radius_scale` (flicker, pulse), `queue_lights` (cull, 64-per-frame cap), `LightingFrame`, `LightStats` |
| `client/src/engine/tween` | `wind_engine_core` | no | `Ease` + `ease()` (easing curves), `Tween` (one float from → to over time) |
| `client/src/engine/text` | `wind_engine_core` | no | `utf8` (glyph boundaries, safe on bad bytes), `layout_text` (wrap at spaces, char-break long words, align, clip with `...`, `truncated` flag) |
| `client/src/engine/ui` (`nine_slice`, `focus_list`, `ui_theme*`) | `wind_engine_core` | no | 9-slice rects, `FocusList` (up/down, wrap, skip disabled), `UiTheme` + `content/ui/theme.json` loader |
| `client/src/engine/ui/ui_painter.*` | `wind_client` | no (uses `Renderer`, `TextRenderer`) | `UiPainter`: panels, buttons, laid-out text with shadow, clipped-text count and one-time warning |
| `client/src/engine/render` (`render_queue`, `render_types`) | `wind_engine_core` | no | draw commands and their sort order |
| `client/src/engine/render/text_renderer.*` | `wind_client` | **yes** (SDL_ttf) | `TextRenderer`: loads TTF fonts (mono hinting), measures and draws one line, caches `TTF_Text` objects (cleared at 512) |
| `client/src/engine/render/renderer.*` | `wind_client` | **yes** | `Renderer`: draws into the 640×360 frame target, blend modes and tint, rects/debug text, owns textures, soft-ellipse textures, screenshots |
| `client/src/engine/resources` | `wind_client` | **yes** (via `Renderer`) | `ResourceManager`: safe paths, cache, fallback texture, stats |
| `client/src/engine/platform` | `wind_client` | **yes** | `SdlPlatform`: SDL and SDL_ttf init, window, `Renderer`, `TextRenderer`, event → `InputState` |
| `client/src/game/input` | `wind_game_core` | no | `Action`, `InputBindings`, `MovementIntent` (8-way), `InputBuffer` |
| `client/src/game/physics` | `wind_game_core` | no | feet box vs tiles: `move_and_slide` (wall sliding, corner assist) |
| `client/src/game/entities` | `wind_game_core` | no | `Mover` (walk, run, dodge), `MovementTuning`, `Stamina` (spend, regen delay, exhaustion), `PlayerMotion` (mover + stamina + dodge buffer in one fixed step), facing helpers |
| `client/src/game/presentation` | `wind_game_core` | no | `DustEmitter`, `AfterimageTrail`, `PlayerSprite` (hero animation, trail, sprite), `AmbientParticles`, `FieldLighting`, `load_field_effects` (effects that only read simulation state) |
| `client/src/game/ui` | `wind_game_core` | no | `AreaBanner` (in → hold → out timeline), `DelayedBar` (trail, smooth fill, auto-hide) |
| `client/src/app` (`field_ui`, `ui_test_panel`, `ui_resources`, `transition_view`) | `wind_client` | no (uses `UiPainter`) | HUD stamina bar, area banner, F7 UI test panel, UI resource loading, fade/iris/notice drawing |
| `client/src/game/world` | `wind_game_core` | no | `GameClock` (time of day), `MapTravel` (cover screen → enter map → uncover, failure notice) |
| `client/src/game/states` | `wind_game_core` (`field_assets`), `wind_client` (`field_state`) | no | `load_field_assets` (reads and checks map, sheet, effects, time, weather); `FieldState` scene: wires simulation, effects, camera, drawing |
| `shared/include/wind/world`, `wind/content` | `wind_shared` | no | map/tileset model, JSON validation (`ErrorList`, `LoadResult`, `JsonReader`, `json_file`), safe content paths, attack and weapon loaders |
| `shared/include/wind/combat` | `wind_shared` | no | `AttackData`, `HitboxShape`, `Weapon`; `compute_damage` and `physical_attack_power` (pure damage functions) |

Rules: game code never includes SDL headers; only `renderer.cpp`, `sdl_platform.cpp` do. The SDL-free libraries are unit-tested without opening a window.

## Frame loop (spec 5.1)

`ClientApp::run` (`client/src/app/client_app.cpp`) runs, per rendered frame:

1. `InputState::begin_frame`, then `SdlPlatform::pump_events` (OS events → keys; focus loss releases all keys)
2. Frame-level actions (Esc quit, F3 debug toggle)
3. `FieldState::handle_input`: movement intent, dodge press into the input buffer, F4 toggle
4. Fixed-step simulation (`FieldState::fixed_update`): `FixedStepClock` at **60 Hz**; a frame counts at most **0.25 s** and runs at most **5** steps, extra time is dropped (no catch-up spiral)
5. Per-frame presentation (`FieldState::frame_update(dt, alpha)`): animation, dust, afterimages, look-ahead, camera follow. Positions are drawn interpolated between the last two simulation steps using the clock's `alpha`, so movement is smooth at any frame rate.
6. Render: scene pushes draw commands into a `RenderQueue` and builds a `LightingFrame`; the queue is sorted; layers `Ground`–`WorldFx` are drawn into the frame target, then `Renderer::apply_lighting`, then `Foreground`–`DebugOverlay`
7. *(audio — later phase)*
8. Debug overlay, then the screenshot flash, drawn last; `end_frame` saves a requested screenshot, then scales the frame target to the window

The render rate follows VSync (60 FPS measured on the dev machine; not guaranteed).

## Coordinates and pixels (spec 5.3, 5.4)

- Simulation positions are world pixels (`Vec2`, floats). `TileCoord` (x right, y down, (0,0) top-left) is used for map lookups and collision.
- `engine/map/coordinates.hpp` is the only tile↔world-pixel conversion; `Camera2D` is the only world↔screen conversion.
- `world_to_tile` uses `floor`, so negative world pixels map to negative tiles correctly.
- The game renders at a fixed **logical resolution of 640×360** into an off-screen **frame target** (`SDL_TEXTUREACCESS_TARGET`). At the end of the frame the target is copied to the window, scaled by an integer factor (`SDL_LOGICAL_PRESENTATION_INTEGER_SCALE`); extra space is letterboxed black. Later phases draw lighting and screen effects onto the target before it is shown.
- Phase 3 check: a frame drawn through the target matched the direct-to-window frame pixel for pixel outside the F3 panel (0 differing pixels; only the live FPS digits differed).
- Textures use nearest-neighbour sampling. The camera's top-left is rounded to whole pixels, and sprite positions are rounded, so pixel art does not shimmer.

## Draw order (spec 5.5)

`RenderQueue` sorts by `(DrawLayer, sort_y, insertion order)` — never by container accident.

| DrawLayer | Contents |
|---|---|
| `Ground` | map layers with `render_layer: "ground"` |
| `GroundDecor` | `"ground_decor"` |
| `Shadows` | soft ellipse under each character's feet |
| `YSorted` | `"low_objects"` tiles **and** characters, afterimages and dust, ordered by their base (feet) y |
| `HighObjects` | `"high_objects"` (e.g. tree canopies; always over characters) |
| `WorldFx` | particles in the air (petals, rain, snow) and fog banks; above canopies |
| *(lighting)* | light map multiplied over everything above (not a queue layer) |
| `Glow` | additive air particles (fireflies, embers), drawn after lighting so they stay bright at night |
| `Foreground` | `"foreground"` |
| `DebugOverlay` | collision overlay, feet box |

A low object's `sort_y` is the bottom edge of its tile; a character's is its feet. A character standing south of a tree trunk is therefore drawn in front of it.

## Content formats

All content is JSON with a `schema_version`, validated before use. Errors name the file and the JSON field path (e.g. `maps/test_meadow.json: layers[1].tiles[37]: tile id 99 is not defined in tileset test_tiles`). Content paths inside JSON are relative to the content root and may not escape it (`..`, absolute paths, drive letters are rejected). Decision record: [0005](DECISIONS/0005-content-formats.md).

### Tileset (`content/tilesets/*.json`)

```json
{ "schema_version": 1, "id": "test_tiles", "image": "tilesets/test_tiles.bmp",
  "tile_width": 32, "tile_height": 32, "columns": 8,
  "tiles": [ { "id": 1, "name": "grass", "walkable": true },
             { "id": 4, "name": "water", "walkable": false,
               "animation": { "frames": [4, 14, 15, 14], "frame_seconds": 0.35 } },
             { "id": 7, "name": "tree_canopy", "walkable": true,
               "sway": { "pixels": 1, "period_seconds": 3.5 } } ] }
```

Tile id `N` is the N-th cell of the image, row by row (id 1 = top-left). Id 0 means "no tile". Parsed by `wind/content/tileset_loader`.

- `animation` (optional, phase 7): 2–16 `frames` (tile ids that must exist in the same tileset) shown in turn for `frame_seconds` (0.05–10) each. Neighbouring cells start on different frames (a fixed stagger from the cell position), so water does not blink in unison. Walkability always comes from the tile id placed in the map, never from a frame.
- `sway` (optional, phase 7): the tile is shifted sideways by up to `pixels` (0–4) on a sine wave with period `period_seconds` (0.5–30), rounded to whole pixels. The wave lags with the cell position, so wind seems to roll across the map. Only the drawn position moves; collision does not.
- Both are presentation only. `engine/map/tile_animation` (`animated_tile`, `sway_offset`) computes them from the field's scene clock (seconds since entering the map).

### Map (`content/maps/*.json`)

```json
{ "schema_version": 1, "map_id": "test_meadow", "display_name": "Test Meadow",
  "tileset": "tilesets/test_tiles.json", "tile_size": 32, "width": 40, "height": 30,
  "spawn_points": [ { "id": "default", "x": 6, "y": 12 }, { "id": "from_cave", "x": 2, "y": 12 } ],
  "exits": [ { "area": { "x": 0, "y": 12, "w": 1, "h": 1 }, "to_map": "maps/test_cave.json", "to_spawn": "entrance",
               "transition": "iris" } ],
  "ambience": {
    "ambient_fx": ["fx/petals.json"],
    "ambient_light": "#1c1a30",
    "player_light": { "radius": 80, "color": "#ffe2b8", "intensity": 0.55 },
    "lights": [ { "x": 3.5, "y": 2.2, "radius": 110, "color": "#ffb060", "intensity": 1,
                  "flicker": 0.35, "pulse": { "frequency": 0.4, "depth": 0.5 }, "fx": "fx/torch_embers.json" } ] },
  "layers": [ { "name": "ground", "render_layer": "ground", "tiles": [ ...width*height ids... ] } ] }
```

- `render_layer`: `ground`, `ground_decor`, `low_objects`, `high_objects`, `foreground`.
- `tiles` is row-major (`index = y * width + x`) and must have exactly `width*height` entries.
- Limits: sides 1–512, 1–8 layers, 1–64 spawn points, tile size 8–128 and equal to the tileset's.
- **Walkability:** a cell is walkable if it has at least one tile and every tile on it is `walkable`. Spawn points must be walkable.
- `exits` (optional, up to 32): `area` in tiles (`x`, `y`, `w`, `h`, inside the map), `to_map` (content path), `to_spawn` (spawn id in that map) and optional `transition` (`fade`, the default, or `iris`). At least one tile of the area must be walkable. The target map and spawn are checked when the hero actually travels; if either is missing, she stays on the current map, the error is logged and a short notice is shown on screen. The test `FieldAssets.EveryContentExitLeadsSomewhereValid` follows every exit in the shipped maps and fails if one points nowhere.
- `ambience` is optional. `ambient_fx` lists up to 8 particle effect files (content paths) that float around the camera on this map. Paths are checked when the map loads; the effect files themselves are validated when the field starts, and a broken effect is logged and skipped.
- `ambient_light` (`#rrggbb`) gives an indoor map a fixed ambient colour (caves). `day_cycle` (content path) makes an outdoor map follow the time of day instead; a map may not have both. `weather` (content path) names the weather table. `night_fx` lists effects that only appear at night (scaled by the night amount). A light with `"night_only": true` brightens with the night amount (street lamps).
- `ambient_light` (`#rrggbb`) turns lighting on for the map; without it the map is drawn fully lit. `player_light` (radius 4–512, colour, intensity 0–1) follows the hero. `lights` (up to 64) each need `x`, `y` (tile units, inside the map), `radius` (4–512 px), `color`, `intensity` (0–1); optional `flicker` (0–1), `pulse` (`frequency` 0–10 Hz, `depth` 0–1) and `fx` (an effect emitted continuously at the light). Parsed by `wind/content/ambience_loader`.

### Particle effect (`content/fx/*.json`)

```json
{ "schema_version": 1, "id": "petals", "shape": "pixel", "blend": "normal",
  "lifetime": [6, 9], "speed": [10, 18], "direction": 110, "spread": 30,
  "gravity": 2, "drag": 0.2, "spawn_radius": 0,
  "start_size": [3, 4], "end_size": 3,
  "start_color": "#f590b0ff", "end_color": "#fbd0dcff", "fade_in": 0.08, "fade_out": 0.2,
  "wobble": { "amplitude": 7, "frequency": 0.5 },
  "burst": 0, "rate": 6 }
```

| Field | Required | Meaning and limits |
|---|---|---|
| `shape` | yes | `soft` (soft circle), `pixel` (crisp square) or `streak` (1 px line along the velocity, length = size; rain) |
| `blend` | yes | `normal`, `add`, `multiply` |
| `lifetime` | yes | seconds, number or `[min, max]`, 0.01–60 |
| `speed` | yes | px/s, number or `[min, max]`, 0–2000 |
| `start_size` | yes | px, number or `[min, max]`, 0–64 |
| `start_color`, `end_color` | yes | `#rrggbb` or `#rrggbbaa`; colour and alpha blend over the lifetime |
| `direction` | no (0) | degrees, 0 = right, 90 = down, 270 = up; callers may override (dust goes backward) |
| `spread` | no (0) | cone width in degrees, 0–360 |
| `gravity`, `drag` | no (0) | px/s² (negative floats up), drag 0–50 per second |
| `spawn_radius` | no (0) | random offset from the spawn point, px |
| `end_size` | no (0) | px at the end of life |
| `fade_in`, `fade_out` | no (0) | fraction of the lifetime spent appearing / disappearing |
| `wobble` | no | `amplitude` px (0–64) and `frequency` Hz (0–20); sideways sway applied when drawing |
| `burst`, `rate` | at least one > 0 | particles per one-shot burst (0–256) / per second when emitting continuously (0–2000) |

Errors carry the file and field path (e.g. `wobble.frequency`). Particles are white textures tinted by the colour: `soft` uses an 8×8 soft circle, `pixel` a 1×1 square scaled up.

### Sprite sheet (`content/sprites/*.json`)

```json
{ "schema_version": 1, "id": "test_hero", "image": "sprites/test_hero.bmp",
  "frame_width": 32, "frame_height": 48, "origin": { "x": 16, "y": 46 },
  "direction_rows": { "down": 0, "left": 1, "right": 2, "up": 3 },
  "clips": { "walk": { "frames": [1, 0, 3, 0], "frame_ms": [100, 100, 100, 100], "loop": true } } }
```

Each direction is a row, each animation frame a column. `origin` is the feet point inside a frame. Frame durations are 1–10000 ms. The player sheet must provide `idle` and `walk`.

### Attack (`content/attacks/*.json`, phase 9, spec 9.2)

```json
{ "schema_version": 1, "id": "longsword_1",
  "windup": 0.08, "active": 0.10, "recovery": 0.18, "cancel_window": [0.18, 0.36],
  "hitbox": { "offset": [18, -8], "size": [28, 24] },
  "power": 1.0, "knockback": 90, "hitstop": 0.05, "shake": 0.15, "next_combo": "longsword_2" }
```

| Field | Required | Range / meaning |
|---|---|---|
| `id` | yes | 1–64 chars |
| `windup`, `active`, `recovery` | yes | seconds: 0–2, 0.01–2, 0–3 (hits land only during `active`) |
| `cancel_window` | yes | `[start, end]` seconds from the attack start, `start <= end <= windup + active + recovery` |
| `hitbox.offset` | yes | `[forward, lift]` px, each −128…128 |
| `hitbox.size` | yes | `[length, width]` px, each 1–256 |
| `power` | yes | 0–10, damage multiplier (spec 9.3) |
| `knockback` | yes | 0–1000 px/s |
| `hitstop`, `shake` | yes | 0–0.5 s, 0–1 (presentation only) |
| `mp_cost` | no (0) | integer 0–999 |
| `fx`, `sfx` | no | content paths (checked for safety; the files are loaded by presentation later) |
| `next_combo` | no | id of the next attack; must match the weapon's list order (below) |

**Hitbox frame.** The box is written for a hero facing **right**: its centre is `forward` px ahead of the feet and `lift` px lower on screen (negative = up, to reach body height); `length` runs along the facing, `width` across it. Facing left mirrors `forward`; facing up/down puts `forward` on the vertical axis and swaps `length`/`width`, while `lift` always stays a screen-vertical shift. (The placement code arrives with the attack runner in phase 9b.) Parsed by `wind/content/attack_loader`.

### Weapon (`content/weapons/*.json`, phase 9, spec 8.4)

```json
{ "schema_version": 1, "id": "longsword", "name": "롱소드", "weight": 6, "base_power": 14,
  "strength_scaling": 0.8, "stamina_per_attack": 14,
  "attacks": ["attacks/longsword_1.json", "attacks/longsword_2.json", "attacks/longsword_3.json"] }
```

- `name` 1–64 bytes (UTF-8), `weight` 0–100, `base_power` 0–999, `strength_scaling` 0–5, `stamina_per_attack` 0–200, optional `crit_bonus` 0–1.
- `attacks`: 1–8 content paths in combo order. Every attack file is loaded and validated with the weapon; one broken file rejects the whole weapon and its own errors are reported too. Attack ids must be unique in the combo.
- `next_combo`, when present on an attack, must equal the id of the next attack in the list; the last attack must not have one. The list order is the source of truth.
- Parsed by `wind/content/weapon_loader` (`parse_weapon` takes an attack source, so tests run without files).

### Images

24-bit BMP, magenta `(255, 0, 255)` is transparent. Test art is generated by `tools/test_content/generate_test_content.ps1` (simple shapes, no third-party assets).

## Resources (spec 5.7)

`ResourceManager::texture(path)`:

- normalizes the path and refuses anything outside the content root;
- loads each file once and returns the cached handle afterwards (counts cache hits);
- on failure logs **one** error with the path and reason, and returns a magenta/black checkerboard placeholder so the game keeps running;
- supports explicit `release` / `release_all`;
- exposes `ResourceStats` for the debug overlay.

Invalid map or sprite *data* stops the client with a non-zero exit code and the full error list; a missing *image* only falls back.

## Input (spec 5.8)

- `SdlPlatform` translates SDL **scancodes** (physical keys, layout-independent) into engine `Key`s; OS key-repeat events are ignored.
- `InputBindings` maps game `Action`s to keys. Defaults: arrows/WASD move, Shift run (held), Space/K/X dodge, Enter confirm (UI), F3 debug overlay, F4 collision overlay, F5 skip time, F6 next weather, F7 UI test panel, F12 screenshot, Esc quit (closes the UI test panel instead when it is open). Left and right Shift both map to `Key::Shift`.
- `MovementIntent` turns held move keys into an 8-way unit vector (diagonals are normalized). When opposite keys are both held, the most recently pressed one wins.
- `InputBuffer` remembers a dodge press for **0.15 s**, so a press slightly before a dodge becomes possible still fires. Presses are recorded per frame and consumed in fixed steps, so a press is never lost on a frame that runs zero steps.
- `InputState::set_ui_capture(true)` makes all game queries return false (for menus and text input, phases 15 and 17). In phase 8 the game loop gives the field an empty input while `FieldUi::captures_input()` (UI test panel open) or a map transition is running; the UI itself reads the real input first.
- Saving custom bindings is not implemented yet.

## Movement (phase 2, running and stamina in phase 8)

Rules and tuning numbers are in [GAMEPLAY.md](GAMEPLAY.md#movement). In short: `Mover` keeps a float feet position and velocity; each 60 Hz step it accelerates toward `input × max_speed`, then `move_and_slide` moves the feet box in sub-steps of at most 1 px, x then y, so the body slides along walls and never tunnels. When moving along one axis and blocked by a corner by a few pixels, corner assist nudges the body sideways. Out-of-map counts as a wall. A dodge moves at a fixed speed for a fixed time, then hands back to walking at max speed and starts a cooldown.

Running (phase 8): `Mover::step(..., run)` raises the top speed to `max_speed × run_multiplier` and reports `State::Running` while moving with run held. `PlayerMotion` owns the `Mover`, the `Stamina` and the dodge `InputBuffer` and decides each step whether a dodge or a run is allowed by stamina (rules and numbers in [GAMEPLAY.md](GAMEPLAY.md#stamina-and-running)). Stamina lives in the field for now and is reset to full when a new map is entered; it moves to the player state when saves arrive (phase 16).

## Text (phase 8, spec 6.9)

- Font: Galmuri11 (regular and bold, OFL 1.1, `content/fonts/`), drawn at **12 px**, the size its pixel grid is designed for. `TextRenderer` sets `TTF_HINTING_MONO`, so glyphs are 1-bit and crisp; positions are rounded to whole pixels.
- `TextRenderer` uses SDL_ttf's renderer text engine (glyph atlas inside SDL_ttf) and keeps one `TTF_Text` per (font, string), cleared when it reaches 512 entries. Input longer than 4096 bytes is cut. If SDL_ttf or a font fails to load, the error is logged and text is simply not drawn.
- `layout_text` (SDL-free) splits on `\n`, wraps at spaces (Korean wraps by word, i.e. 어절), breaks a single word longer than the box by UTF-8 glyph, aligns left/center/right (center rounds down to a whole pixel) and, when `max_lines` or the box height is exceeded, ends the last line with `...` and sets `truncated`.
- **Clipped-text check:** `UiPainter::text` counts truncated boxes per frame (shown on F3) and logs `UI text clipped (box WxH): <text>` once per distinct string.

## UI (phase 8, spec 6.9, 13.3)

UI theme (`content/ui/theme.json`):

```json
{ "schema_version": 1, "image": "ui/ui_frames.bmp",
  "fonts": { "regular": { "file": "fonts/Galmuri11.ttf", "size": 12 },
             "bold": { "file": "fonts/Galmuri11-Bold.ttf", "size": 12 } },
  "panel": { "source": [0, 0, 24, 24], "border": 8 },
  "button": { "source": [24, 0, 24, 16], "border": 5 },
  "button_focused": { "source": [48, 0, 24, 16], "border": 5 },
  "panel_alpha": 235,
  "colors": { "text": "#f2efe6", "text_dim": "#9aa3b5", "text_focused": "#ffe9a8", "text_shadow": "#10131c" } }
```

- Paths must stay inside the content root; `source` is `[x, y, w, h]` with w, h ≥ 1 and each value ≤ 1024; `border` may not exceed half the frame's width or height; font size 6–64; colours `#rrggbb[aa]` (optional, defaults in `UiTheme`); `panel_alpha` 0–255. A broken theme is logged and the UI falls back to flat rectangles with no text.
- `nine_slice` keeps corners at their pixel size, stretches edges along one axis and the centre both ways, and shrinks the border when the target is smaller than two borders.
- `FocusList` moves focus up/down with wrap-around and skips disabled items. The F7 UI test panel (debug) shows a panel, a bold title, a wrapped paragraph, a deliberately clipped line and three buttons; ↑/↓ (or W/S) move focus, Enter presses, Esc or F7 closes. While it is open the field receives no input.
- `AreaBanner`: on entering a map (start and every successful travel) the map's `display_name` appears at the top: 0.5 s in (`out_cubic`, sliding down 8 px), 2.2 s hold, 0.7 s out (`in_quad`).
- `DelayedBar`: a drop shows instantly, the trail waits 0.35 s then shrinks at 1.2 bar/s; a rise fills at 2.5 bar/s; with auto-hide the bar dims to 30 % after 1.2 s at full. HP and MP bars (phases 9 and 12) reuse it.
- Draw order after lighting: world `Glow…DebugOverlay` → HUD + banner → transition cover → travel notice → UI test panel → F3 panel → screenshot flash.
- Test-only `--hold <key>@<from>-<to>` holds a key for a range of frames (same key names as `--press`, plus `shift`, `enter`, `esc`, `f7`).

## Effects (phases 2 and 4)

Values are listed in [VISUALS.md](VISUALS.md).

- **Shadow:** a soft black ellipse under the feet (`Shadows` layer).
- **Dust:** `DustEmitter` decides *when* (every 14 px walked above 40 px/s, and when a dodge starts); `fx/dust_walk.json` and `fx/dust_dodge.json` decide *how it looks*. Dust lives in the ground pool (1024 slots, `YSorted` by feet y).
- **Map ambience:** `AmbientParticles` runs one `ParticleEmitter` per `ambient_fx` effect, spawning inside the camera view plus a 48 px margin. These live in the air pool (4000 slots, `WorldFx` layer).
- **Stress check:** `--fx-stress N` (0–4000) adds a test effect that keeps about N particles alive. Phase 4 measurement: 2039 particles, 60.0 FPS (VSync), 16.66 ms per frame, 1654 draw calls, 0 dropped.
- **Afterimages:** `AfterimageTrail` snapshots the sprite every 0.035 s while dodging; snapshots fade out over 0.18 s.
- **Camera look-ahead:** the camera target is shifted up to 28 px toward the movement direction, smoothed.

Effects read simulation state and events only; they never move the character.

## Map travel (spec 7.4, phase 7)

- `FieldState` checks the feet tile after every simulation step. An exit fires only after the hero has stood on a non-exit tile once ("armed"), so arriving next to or on an exit never bounces her straight back.
- `game::MapTravel` (SDL-free, `game/world/map_travel`) runs the sequence. `ClientApp` passes it the field's exit request; it covers the screen with `ScreenFade` (0.35 s, `in_quad`) in the exit's `transition` style, calls the `EnterMap` callback once when fully covered, then uncovers (0.35 s, `out_quad`). Input is ignored while busy. The F3 panel stays visible on top.
- `EnterMap` builds the new `FieldState` for `to_map` / `to_spawn` and swaps it in only on success. On failure the current field is kept, the reason is logged, and `MapTravel::notice()` holds a short English message for 4 s that `ClientApp` draws in red at the bottom of the screen (debug font until the phase 8 text renderer). Tested in `map_travel_tests.cpp` and `field_assets_tests.cpp`.
- **Fade** draws one translucent black rectangle. **Iris** (`engine/fx/iris_mask`) leaves a round hole centred on the hero's body (`FieldState::focus_on_screen`) and covers the rest with opaque black: the hole radius goes from the distance to the farthest screen corner down to 0, and the cover is built as one rectangle per pixel row on each side of the hole (plus one block above and below), so the edge is pixel-exact. When opening, the hole is centred on the hero in the new map.
- Each `FieldState` destroys the textures it generated (shadow, particles, light) when it is replaced.
- Maps: meadow ↔ cave (iris, left end of the meadow path) and meadow ↔ forest (fade, right end of the meadow path).
- Test-only `--auto-move <dir>` holds a direction for the whole run; with `--auto-move left` the hero walks from the meadow start into the cave, with `--auto-move right` into the forest.

## Time of day (spec 6.4, phase 6)

- `GameClock` (`client/src/game/world`) runs in the fixed simulation step at **60 real seconds per game hour**, starting at 09:00 (`--time` overrides). It lives in `ClientApp`, not in a map, so time continues across map travel. F5 skips 3 hours.
- Day cycle file (`content/time/*.json`): `{ "schema_version": 1, "keys": [ { "hour": 0–23.999, "ambient": "#rrggbb", "night": 0–1 }, ... ] }`, 2–32 keys with increasing hours. `sample_day_cycle` blends the two keys around the hour and wraps past midnight (last key → first key of the next day).
- Outdoor ambient = day cycle colour × weather tint (per channel). Indoor maps (`ambient_light`) ignore time and weather.

## Weather (spec 6.5, phase 6)

- Weather file (`content/weather/*.json`): `transition_seconds` (0.1–60) and 1–16 `states`, each with a unique `id`, `weight` (0–100, at least one > 0), `duration` (seconds, number or `[min, max]`, 1–3600), optional `fx` and `ground_fx` (up to 4 effect paths each), `tint` (`#rrggbb`) and `fog` (0–1).
- `WeatherSystem` picks states by weight (random seed per run). The first state starts at full strength (`--weather` picks it). On a change the current state fades to 0 over `transition_seconds`, then the next one fades in. F6 starts the next state in table order and uses a quick 0.4 s fade each way so the result shows at once; the F3 panel shows a change in progress as `old -> new`, and the log records it.
- Intensity scales the state's emitters (`ParticleEmitter::update(..., rate_scale)`), the tint (blended from white), and fog. Sky effects spawn around the view in the air pool; ground effects (splashes) spawn inside the view in the ground pool.
- `FogLayer`: 7 large soft banks (the light texture) drifting sideways in screen space, alpha up to 120 × fog, drawn on `WorldFx` before lighting so fog darkens at night.
- `FieldAtmosphere` combines the day cycle, weather, fog and night effects and hands `{ambient, night}` to `FieldLighting::set_conditions` each frame.

## Lighting (spec 6.3, phase 5)

Decision and limits: [ADR 0008](DECISIONS/0008-lighting-with-sdl-renderer.md).

- `FieldLighting` turns map light definitions into `Light`s (tile units → pixels, a different flicker phase per light), moves the hero light (14 px above the feet), and runs one `ParticleEmitter` per light `fx` (embers go to the air pool).
- Each frame `queue_lights` keeps lights whose radius box touches the view, up to `kMaxLightsPerFrame` = 64 (hero light first), and turns them into `ADD` sprites of a 128 px radial light texture (linear filtering).
- `Renderer::apply_lighting`: clear the light map to the ambient colour → draw the light sprites → multiply the light map over the frame target (`MOD`).
- **Flicker:** strength × (1 − flicker × w), where w mixes two sine waves (9.1 and 15.7 rad/s); the radius also shrinks by up to 12 % of the flicker amount. **Pulse:** strength × (1 − depth × (0.5 + 0.5 sin(2π f t + phase))).
- F3 shows `lights / drawn / offscreen / over cap`, or `lighting off`. Another line shows the time, night amount and weather (or "indoors").
- `--light-stress N` (0–64) adds N lights around the start on a lit map. Phase 5 measurement: 42 lights (37 drawn), 60.0 FPS, 16.67 ms.

## Blend modes and tint (spec 5.2)

`SpriteDraw` has `tint` (colour multiply + opacity in `a`) and `blend` (`Normal`, `Add`, `Multiply`). The renderer sets colour mod, alpha mod and blend mode on the texture for every draw. Uses and values: [VISUALS.md](VISUALS.md#blend-modes).

## Tweens (spec 5.6)

`Tween(from, to, seconds, ease)` changes one float over time; `update(dt)` clamps at the end and ignores negative time; a zero-length tween is finished immediately. `ease()` clamps `t` to 0–1, returns 0 at the start and 1 at the end for every curve (`out_back` and `out_elastic` overshoot in between). `parse_ease` maps data names such as `"out_back"`.

## Screenshots

F12, or `--screenshot-at N` (repeatable) for automated checks, saves the frame target as a 640×360 BMP named `wind_YYYYMMDD_HHMMSS_mmm.bmp` (local time). The folder is `--screenshot-dir`, or `%APPDATA%\ProjectWIND\screenshots` (`SdlPlatform::user_data_dir`, via `SDL_GetPrefPath`). Pixels are read in `end_frame` before presenting. A white flash and a "screenshot saved" label follow on the next frames.

## Debug overlay (spec 5.10)

F3 toggles: FPS and average frame time (0.5 s window), dropped-time frames, map id, feet position (px) and tile, movement state (idle/walking/running/dodging), facing, speed, stamina (and "exhausted"), draw calls, culled sprites, tiles queued, particle count and drops, texture count, cache hits, load failures, UI text draws / cached texts / clipped texts this frame, and two lines of key help. F4 toggles the red collision overlay and the cyan feet box. Since phase 8 the panel starts **hidden**, because the HUD now sits in the top-left corner (`--press f3@2` shows it in scripted runs).

## Known limitations

- Images are BMP only (no PNG decoder dependency yet).
- The F3 panel still uses SDL's built-in 8×8 ASCII debug font; game UI text uses Galmuri11 through `TextRenderer`.
- UI strings (UI test panel, travel notice) are still written in code; spec 6.9 wants them in a data file, which comes with the first real menus (phase 15).
- Text is laid out by our code and drawn line by line; no right-to-left or complex shaping (HarfBuzz is off), which Korean Hangul and Latin do not need.
- No audio yet (spec 5.9).
- UI and world share the logical resolution; a separate UI scale comes with real UI work.
- Key binding persistence is not implemented.
- Sprites have 4 directions; 8-way movement picks the nearest of the four (`facing_from_vector`). There is no dedicated dodge animation yet (the walk clip plays at 2× speed).
- Movement tuning lives in code (`MovementTuning`) until a character data file exists.
