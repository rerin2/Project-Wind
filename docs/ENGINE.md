# Engine Design (client)

Status: phase 4 (data-driven particles: effect files, emitters, map ambience). Spec reference: master spec section 5.

The client engine is our own code on top of SDL3. SDL3 is used only for the window, OS events, the GPU-backed 2D renderer, and BMP decoding. Everything else (loop, map, camera, animation, input model, particles, resources, debug view) lives in this repository.

## Module layout

| Folder | Library | SDL? | Responsibility |
|---|---|---|---|
| `client/src/engine/core` | `wind_engine_core` | no | `Vec2`/`RectF` math, `FixedStepClock` |
| `client/src/engine/map` | `wind_engine_core` | no | tile↔world coordinates, visible-tile culling, tile batching into the render queue |
| `client/src/engine/camera` | `wind_engine_core` | no | `Camera2D`: world↔screen, follow, bounds clamp, pixel snapping; `LookAhead`: smoothed offset toward the movement direction |
| `client/src/engine/input` | `wind_engine_core` | no | `InputState`: held/pressed/released, UI capture flag |
| `client/src/engine/animation` | `wind_engine_core` | no | `AnimationClip`, `SpriteSheet`, `Animator` (one file each), sprite sheet JSON loader |
| `client/src/engine/fx` | `wind_engine_core` | no | `Particle`, `ParticleEffect` + loader, `ParticlePool` (fixed capacity, drop counter), `ParticleRandom`, `make_particle`/`emit_burst`, `ParticleEmitter` (continuous), `queue_particles` |
| `client/src/engine/tween` | `wind_engine_core` | no | `Ease` + `ease()` (easing curves), `Tween` (one float from → to over time) |
| `client/src/engine/render` (`render_queue`, `render_types`) | `wind_engine_core` | no | draw commands and their sort order |
| `client/src/engine/render/renderer.*` | `wind_client` | **yes** | `Renderer`: draws into the 640×360 frame target, blend modes and tint, rects/debug text, owns textures, soft-ellipse textures, screenshots |
| `client/src/engine/resources` | `wind_client` | **yes** (via `Renderer`) | `ResourceManager`: safe paths, cache, fallback texture, stats |
| `client/src/engine/platform` | `wind_client` | **yes** | `SdlPlatform`: SDL init, window, event → `InputState` |
| `client/src/game/input` | `wind_game_core` | no | `Action`, `InputBindings`, `MovementIntent` (8-way), `InputBuffer` |
| `client/src/game/physics` | `wind_game_core` | no | feet box vs tiles: `move_and_slide` (wall sliding, corner assist) |
| `client/src/game/entities` | `wind_game_core` | no | `Mover` (speed, acceleration, dodge), `MovementTuning`, facing helpers |
| `client/src/game/presentation` | `wind_game_core` | no | `DustEmitter`, `AfterimageTrail` (effects that only read simulation state) |
| `client/src/game/states` | `wind_client` | no | `FieldState` scene: wires simulation, effects, camera, drawing |
| `shared/include/wind/world`, `wind/content` | `wind_shared` | no | map/tileset model, JSON validation (`ErrorList`, `LoadResult`, `JsonReader`, `json_file`), safe content paths |

Rules: game code never includes SDL headers; only `renderer.cpp`, `sdl_platform.cpp` do. The SDL-free libraries are unit-tested without opening a window.

## Frame loop (spec 5.1)

`ClientApp::run` (`client/src/app/client_app.cpp`) runs, per rendered frame:

1. `InputState::begin_frame`, then `SdlPlatform::pump_events` (OS events → keys; focus loss releases all keys)
2. Frame-level actions (Esc quit, F3 debug toggle)
3. `FieldState::handle_input`: movement intent, dodge press into the input buffer, F4 toggle
4. Fixed-step simulation (`FieldState::fixed_update`): `FixedStepClock` at **60 Hz**; a frame counts at most **0.25 s** and runs at most **5** steps, extra time is dropped (no catch-up spiral)
5. Per-frame presentation (`FieldState::frame_update(dt, alpha)`): animation, dust, afterimages, look-ahead, camera follow. Positions are drawn interpolated between the last two simulation steps using the clock's `alpha`, so movement is smooth at any frame rate.
6. Render: scene pushes draw commands into a `RenderQueue`, the queue is sorted, `Renderer::submit` draws it into the frame target
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
| `WorldFx` | particles in the air (map ambience: petals, fireflies); above canopies |
| `Foreground` | `"foreground"` |
| `DebugOverlay` | collision overlay, feet box |

A low object's `sort_y` is the bottom edge of its tile; a character's is its feet. A character standing south of a tree trunk is therefore drawn in front of it.

## Content formats

All content is JSON with a `schema_version`, validated before use. Errors name the file and the JSON field path (e.g. `maps/test_meadow.json: layers[1].tiles[37]: tile id 99 is not defined in tileset test_tiles`). Content paths inside JSON are relative to the content root and may not escape it (`..`, absolute paths, drive letters are rejected). Decision record: [0005](DECISIONS/0005-content-formats.md).

### Tileset (`content/tilesets/*.json`)

```json
{ "schema_version": 1, "id": "test_tiles", "image": "tilesets/test_tiles.bmp",
  "tile_width": 32, "tile_height": 32, "columns": 8,
  "tiles": [ { "id": 1, "name": "grass", "walkable": true } ] }
```

Tile id `N` is the N-th cell of the image, row by row (id 1 = top-left). Id 0 means "no tile".

### Map (`content/maps/*.json`)

```json
{ "schema_version": 1, "map_id": "test_meadow", "display_name": "Test Meadow",
  "tileset": "tilesets/test_tiles.json", "tile_size": 32, "width": 40, "height": 30,
  "spawn_points": [ { "id": "default", "x": 6, "y": 12 } ],
  "exits": [],
  "ambience": { "ambient_fx": ["fx/petals.json", "fx/fireflies.json"] },
  "layers": [ { "name": "ground", "render_layer": "ground", "tiles": [ ...width*height ids... ] } ] }
```

- `render_layer`: `ground`, `ground_decor`, `low_objects`, `high_objects`, `foreground`.
- `tiles` is row-major (`index = y * width + x`) and must have exactly `width*height` entries.
- Limits: sides 1–512, 1–8 layers, 1–64 spawn points, tile size 8–128 and equal to the tileset's.
- **Walkability:** a cell is walkable if it has at least one tile and every tile on it is `walkable`. Spawn points must be walkable.
- `exits` is accepted (must be an array) but not used until map transitions are implemented.
- `ambience` is optional. `ambient_fx` lists up to 8 particle effect files (content paths) that float around the camera on this map. Paths are checked when the map loads; the effect files themselves are validated when the field starts, and a broken effect is logged and skipped.

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
| `shape` | yes | `soft` (soft circle) or `pixel` (crisp square) |
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
| `burst`, `rate` | at least one > 0 | particles per one-shot burst (0–256) / per second when emitting continuously (0–500) |

Errors carry the file and field path (e.g. `wobble.frequency`). Particles are white textures tinted by the colour: `soft` uses an 8×8 soft circle, `pixel` a 1×1 square scaled up.

### Sprite sheet (`content/sprites/*.json`)

```json
{ "schema_version": 1, "id": "test_hero", "image": "sprites/test_hero.bmp",
  "frame_width": 32, "frame_height": 48, "origin": { "x": 16, "y": 46 },
  "direction_rows": { "down": 0, "left": 1, "right": 2, "up": 3 },
  "clips": { "walk": { "frames": [1, 0, 3, 0], "frame_ms": [100, 100, 100, 100], "loop": true } } }
```

Each direction is a row, each animation frame a column. `origin` is the feet point inside a frame. Frame durations are 1–10000 ms. The player sheet must provide `idle` and `walk`.

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
- `InputBindings` maps game `Action`s to keys. Defaults: arrows/WASD move, Space/K/X dodge, F3 debug overlay, F4 collision overlay, F12 screenshot, Esc quit.
- `MovementIntent` turns held move keys into an 8-way unit vector (diagonals are normalized). When opposite keys are both held, the most recently pressed one wins.
- `InputBuffer` remembers a dodge press for **0.15 s**, so a press slightly before a dodge becomes possible still fires. Presses are recorded per frame and consumed in fixed steps, so a press is never lost on a frame that runs zero steps.
- `InputState::set_ui_capture(true)` makes all game queries return false (for menus and text input, phases 15 and 17).
- Saving custom bindings is not implemented yet.

## Movement (phase 2)

Rules and tuning numbers are in [GAMEPLAY.md](GAMEPLAY.md#movement). In short: `Mover` keeps a float feet position and velocity; each 60 Hz step it accelerates toward `input × max_speed`, then `move_and_slide` moves the feet box in sub-steps of at most 1 px, x then y, so the body slides along walls and never tunnels. When moving along one axis and blocked by a corner by a few pixels, corner assist nudges the body sideways. Out-of-map counts as a wall. A dodge moves at a fixed speed for a fixed time, then hands back to walking at max speed and starts a cooldown.

## Effects (phases 2 and 4)

Values are listed in [VISUALS.md](VISUALS.md).

- **Shadow:** a soft black ellipse under the feet (`Shadows` layer).
- **Dust:** `DustEmitter` decides *when* (every 14 px walked above 40 px/s, and when a dodge starts); `fx/dust_walk.json` and `fx/dust_dodge.json` decide *how it looks*. Dust lives in the ground pool (1024 slots, `YSorted` by feet y).
- **Map ambience:** `AmbientParticles` runs one `ParticleEmitter` per `ambient_fx` effect, spawning inside the camera view plus a 48 px margin. These live in the air pool (4000 slots, `WorldFx` layer).
- **Stress check:** `--fx-stress N` (0–4000) adds a test effect that keeps about N particles alive. Phase 4 measurement: 2039 particles, 60.0 FPS (VSync), 16.66 ms per frame, 1654 draw calls, 0 dropped.
- **Afterimages:** `AfterimageTrail` snapshots the sprite every 0.035 s while dodging; snapshots fade out over 0.18 s.
- **Camera look-ahead:** the camera target is shifted up to 28 px toward the movement direction, smoothed.

Effects read simulation state and events only; they never move the character.

## Blend modes and tint (spec 5.2)

`SpriteDraw` has `tint` (colour multiply + opacity in `a`) and `blend` (`Normal`, `Add`, `Multiply`). The renderer sets colour mod, alpha mod and blend mode on the texture for every draw. Uses and values: [VISUALS.md](VISUALS.md#blend-modes).

## Tweens (spec 5.6)

`Tween(from, to, seconds, ease)` changes one float over time; `update(dt)` clamps at the end and ignores negative time; a zero-length tween is finished immediately. `ease()` clamps `t` to 0–1, returns 0 at the start and 1 at the end for every curve (`out_back` and `out_elastic` overshoot in between). `parse_ease` maps data names such as `"out_back"`.

## Screenshots

F12, or `--screenshot-at N` (repeatable) for automated checks, saves the frame target as a 640×360 BMP named `wind_YYYYMMDD_HHMMSS_mmm.bmp` (local time). The folder is `--screenshot-dir`, or `%APPDATA%\ProjectWIND\screenshots` (`SdlPlatform::user_data_dir`, via `SDL_GetPrefPath`). Pixels are read in `end_frame` before presenting. A white flash and a "screenshot saved" label follow on the next frames.

## Debug overlay (spec 5.10)

F3 toggles: FPS and average frame time (0.5 s window), dropped-time frames, map id, feet position (px) and tile, movement state (idle/walking/dodging), facing, speed, draw calls, culled sprites, tiles queued, particle count and drops, texture count, cache hits, load failures. F4 toggles the red collision overlay and the cyan feet box. The panel is visible when the game starts.

## Known limitations

- Images are BMP only (no PNG decoder dependency yet).
- The debug font is SDL's built-in 8×8 ASCII font; Korean text needs a real font renderer (later phase).
- No audio yet (spec 5.9).
- UI and world share the logical resolution; a separate UI scale comes with real UI work.
- Key binding persistence is not implemented.
- Sprites have 4 directions; 8-way movement picks the nearest of the four (`facing_from_vector`). There is no dedicated dodge animation yet (the walk clip plays at 2× speed).
- Movement tuning lives in code (`MovementTuning`) until a character data file exists.
