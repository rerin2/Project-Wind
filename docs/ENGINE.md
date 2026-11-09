# Engine Design (client)

Status: phase 2 (free movement, dodge, first effects). Spec reference: master spec section 5.

The client engine is our own code on top of SDL3. SDL3 is used only for the window, OS events, the GPU-backed 2D renderer, and BMP decoding. Everything else (loop, map, camera, animation, input model, particles, resources, debug view) lives in this repository.

## Module layout

| Folder | Library | SDL? | Responsibility |
|---|---|---|---|
| `client/src/engine/core` | `wind_engine_core` | no | `Vec2`/`RectF` math, `FixedStepClock` |
| `client/src/engine/map` | `wind_engine_core` | no | tile↔world coordinates, visible-tile culling, tile batching into the render queue |
| `client/src/engine/camera` | `wind_engine_core` | no | `Camera2D`: world↔screen, follow, bounds clamp, pixel snapping; `LookAhead`: smoothed offset toward the movement direction |
| `client/src/engine/input` | `wind_engine_core` | no | `InputState`: held/pressed/released, UI capture flag |
| `client/src/engine/animation` | `wind_engine_core` | no | `AnimationClip`, `SpriteSheet`, `Animator` (one file each), sprite sheet JSON loader |
| `client/src/engine/fx` | `wind_engine_core` | no | `Particle`, `ParticlePool` (fixed capacity, drag, gravity, drop counter), `queue_particles` |
| `client/src/engine/render` (`render_queue`, `render_types`) | `wind_engine_core` | no | draw commands and their sort order |
| `client/src/engine/render/renderer.*` | `wind_client` | **yes** | `Renderer`: draws sprites/rects/debug text, owns textures, makes soft-ellipse textures (shadow, dust) |
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
6. Render: scene pushes draw commands into a `RenderQueue`, the queue is sorted, `Renderer::submit` draws it
7. *(audio — later phase)*
8. Debug overlay drawn last

The render rate follows VSync (60 FPS measured on the dev machine; not guaranteed).

## Coordinates and pixels (spec 5.3, 5.4)

- Simulation positions are world pixels (`Vec2`, floats). `TileCoord` (x right, y down, (0,0) top-left) is used for map lookups and collision.
- `engine/map/coordinates.hpp` is the only tile↔world-pixel conversion; `Camera2D` is the only world↔screen conversion.
- `world_to_tile` uses `floor`, so negative world pixels map to negative tiles correctly.
- The game renders at a fixed **logical resolution of 640×360**, scaled by an integer factor to the window (`SDL_LOGICAL_PRESENTATION_INTEGER_SCALE`); extra space is letterboxed.
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
  "layers": [ { "name": "ground", "render_layer": "ground", "tiles": [ ...width*height ids... ] } ] }
```

- `render_layer`: `ground`, `ground_decor`, `low_objects`, `high_objects`, `foreground`.
- `tiles` is row-major (`index = y * width + x`) and must have exactly `width*height` entries.
- Limits: sides 1–512, 1–8 layers, 1–64 spawn points, tile size 8–128 and equal to the tileset's.
- **Walkability:** a cell is walkable if it has at least one tile and every tile on it is `walkable`. Spawn points must be walkable.
- `exits` is accepted (must be an array) but not used until map transitions are implemented.

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
- `InputBindings` maps game `Action`s to keys. Defaults: arrows/WASD move, Space/K/X dodge, F3 debug overlay, F4 collision overlay, Esc quit.
- `MovementIntent` turns held move keys into an 8-way unit vector (diagonals are normalized). When opposite keys are both held, the most recently pressed one wins.
- `InputBuffer` remembers a dodge press for **0.15 s**, so a press slightly before a dodge becomes possible still fires. Presses are recorded per frame and consumed in fixed steps, so a press is never lost on a frame that runs zero steps.
- `InputState::set_ui_capture(true)` makes all game queries return false (for a chat box, phase 5).
- Saving custom bindings is not implemented yet.

## Movement (phase 2)

Rules and tuning numbers are in [GAMEPLAY.md](GAMEPLAY.md#movement). In short: `Mover` keeps a float feet position and velocity; each 60 Hz step it accelerates toward `input × max_speed`, then `move_and_slide` moves the feet box in sub-steps of at most 1 px, x then y, so the body slides along walls and never tunnels. When moving along one axis and blocked by a corner by a few pixels, corner assist nudges the body sideways. Out-of-map counts as a wall. A dodge moves at a fixed speed for a fixed time, then hands back to walking at max speed and starts a cooldown.

## Effects (phase 2)

Values are listed in [VISUALS.md](VISUALS.md).

- **Shadow:** a soft black ellipse under the feet (`Shadows` layer).
- **Dust:** `DustEmitter` puffs every 14 px walked (above 40 px/s) and bursts 10 puffs when a dodge starts. Particles live in a 512-slot `ParticlePool`.
- **Afterimages:** `AfterimageTrail` snapshots the sprite every 0.035 s while dodging; snapshots fade out over 0.18 s.
- **Camera look-ahead:** the camera target is shifted up to 28 px toward the movement direction, smoothed.

Effects read simulation state and events only; they never move the character.

## Debug overlay (spec 5.10)

F3 toggles: FPS and average frame time (0.5 s window), dropped-time frames, map id, feet position (px) and tile, movement state (idle/walking/dodging), facing, speed, draw calls, culled sprites, tiles queued, particle count and drops, texture count, cache hits, load failures. F4 toggles the red collision overlay and the cyan feet box.

## Known limitations

- Images are BMP only (no PNG decoder dependency yet).
- The debug font is SDL's built-in 8×8 ASCII font; Korean text needs a real font renderer (later phase).
- No audio yet (spec 5.9).
- UI and world share the logical resolution; a separate UI scale comes with real UI work.
- Key binding persistence is not implemented.
- Sprites have 4 directions; 8-way movement picks the nearest of the four (`facing_from_vector`). There is no dedicated dodge animation yet (the walk clip plays at 2× speed).
- Movement tuning lives in code (`MovementTuning`) until a character data file exists.
