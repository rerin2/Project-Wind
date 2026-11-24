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
| `client/src/engine/render/renderer.*` | `wind_client` | **yes** | `Renderer`: draws into the 640×360 frame target, blend modes and tint, rects/debug text, owns textures, soft-ellipse, light and vignette textures, screenshots, `apply_desaturation` (12b grayscale; owns a 640×360 RGBA32 streaming scratch texture). `frame_readback.*` (free helpers, SDL layer): `save_frame_bmp`, `desaturate_frame` (read target → RGBA32 → `desaturate_rgba` → upload → draw back). `desaturate.*` (no SDL, `wind_engine_core`): `desaturate_rgba` on RGBA bytes |
| `client/src/engine/resources` | `wind_client` | **yes** (via `Renderer`) | `ResourceManager`: safe paths, cache, fallback texture, stats |
| `client/src/engine/platform` | `wind_client` | **yes** | `SdlPlatform`: SDL and SDL_ttf init, window, `Renderer`, `TextRenderer`, event → `InputState` |
| `client/src/game/input` | `wind_game_core` | no | `Action`, `InputBindings`, `MovementIntent` (8-way), `InputBuffer` |
| `client/src/game/physics` | `wind_game_core` | no | feet box vs tiles: `move_and_slide` (wall sliding, corner assist); `separate_bodies` (overlapping body circles, weighted shares, per-step force limit) |
| `client/src/game/entities` | `wind_game_core` | no | `Mover` (walk, run, dodge), `MovementTuning`, `Stamina` (spend, regen delay, exhaustion), `Mana` (MP, all-or-nothing spend, steady regen), `Health` (HP with a floor), `HitReaction` (stagger, invulnerability, knockback that slows to 0 over the stagger), `CharacterState` + `can_transition` (state table), `PlayerMotion` (mover + stamina + MP + HP + hit reaction + attack + cast + dodge/attack/cast buffer in one fixed step; `take_hit`, `can_be_hit`, `shove`), facing helpers |
| `client/src/game/combat` | `wind_game_core` | no | `AttackRunner` (windup → active → recovery, cancel window, combo order, one hit per target per swing), `place_hitbox` (facing-aware hitbox rect), `TrainingDummy` (HP, knockdown, recovery), `resolve_swing` (hitbox ∩ hurtbox → one hit per swing → `compute_damage`; one template shared by dummies and monsters), `SpellCaster` (cast timer; a beam spell charges, then stays on while the key is held, 15d), `CombatRandom` (seeded splitmix64 for crits), `KnockSway` (spring knockback for planted props), `HitStop` (hit freeze → simulation `time_scale`), `SlowMotion` (12b fall slow-down 0.3 → 1), `SimTimeScale` (hitstop × slow motion, the one simulation time scale), `Projectile` (flight in sub-steps of half its size; target → wall → range; entity-only tiles are not walls), `HeroMagic` (hero spells after release: shards, star swarm, moon blade, 15c), `HeroBeam` (comet beam band cut at walls, strikes everything inside every `tick_seconds`, 15d) |
| `client/src/game/monsters` | `wind_game_core` | no | `Monster` (one monster: home, idle ↔ wander with the seeded `CombatRandom`, notice → chase → windup → lunge → recover → return, leash and territory rules; HP, hurt stagger, knockback, death and `gone`), `MonsterState`, `step_toward` / `step_along` (one step of walking or lunging with wall sliding) |
| `client/src/game/presentation` | `wind_game_core` | no | `DustEmitter`, `AfterimageTrail`, `PlayerSprite` (hero animation, trail, sprite), `CombatView` (dummies, HP bars, hit flash, F4 hit/hurt boxes), `queue_enemy_bar` (shared enemy HP bar), `MonsterView` + `slime_pose` / `slime_action_pose` (monster shadow, breathing, hopping, notice hop, windup crouch and shiver, lunge arc, landing splat, F4 boxes incl. attack hitbox; hit flash, HP bar, hurt recoil, death melt and blink) + `monster_marks` ("!" mark, red ground telegraph and its red light), `HurtVignette` (red screen edges on a hero hit), `hero_blink_alpha` (invulnerability blink), `SlashArc` (swing arc), `SpellView` (cast glow, shards, trail/burst, shard lights, F4 shard boxes), `BeamView` (comet beam beads, flares, sparkles, three lights, F4 band, 15d), `FieldCombatFx` (owns the combat views; gathers hits per step and plays flash, shake, numbers, sparks per frame), `ScreenShake` (trauma), `DamageNumbers` (pop and fade), `FeelSettings`, `FieldCamera` (follow + look-ahead + shake on a drawing copy), `queue_shadow`, `AmbientParticles`, `FieldLighting` (also takes moving lights each frame), `load_field_effects` (effects that only read simulation state) |
| `client/src/game/ui` | `wind_game_core` | no | `AreaBanner` (in → hold → out timeline), `DelayedBar` (trail, smooth fill, auto-hide) |
| `client/src/app` (`field_ui`, `ui_test_panel`, `ui_resources`, `transition_view`, `damage_number_drawing`) | `wind_client` | no (uses `UiPainter`) | HUD stamina and MP bars, damage numbers, area banner, F7 UI test panel, UI resource loading, fade/iris/notice drawing |
| `client/src/game/world` | `wind_game_core` | no | `GameClock` (time of day), `MapTravel` (cover screen → enter map → uncover, failure notice) |
| `client/src/game/states` | `wind_game_core` (`field_assets`, `field_combat`), `wind_client` (`field_state`) | no | `load_field_assets` (reads and checks map, sheet, effects, time, weather, weapon, dummies, monsters); `FieldCombat` (field dummies and monsters, one hit check per fixed step, last hit; monster dice are a separate stream from crit dice; monster lunges hit the hero (`HeroHitEvent`); bodies pushed apart; `strongest_hitstop`); `FieldState` scene: wires simulation, effects, camera, drawing |
| `shared/include/wind/world`, `wind/content` | `wind_shared` | no | map/tileset model, JSON validation (`ErrorList`, `LoadResult`, `JsonReader`, `json_file`), safe content paths, attack, weapon, spell, training dummy and monster loaders, shared `combat_readers` (hitbox, projectile) and `body_readers` (hurtbox, sprite strip) |
| `shared/include/wind/combat` | `wind_shared` | no | `AttackData`, `HitboxShape`, `HurtboxShape`, `SpriteStrip`, `TrainingDummyData`, `MonsterData`, `Weapon`, `Spell` (+ `BeamData`, `SpellLight`), `ProjectileData`; `compute_damage` and `physical_attack_power` (pure damage functions) |

Rules: game code never includes SDL headers; only `renderer.cpp`, `sdl_platform.cpp` do. The SDL-free libraries are unit-tested without opening a window.

## Frame loop (spec 5.1)

`ClientApp::run` (`client/src/app/client_app.cpp`) runs, per rendered frame:

1. `InputState::begin_frame`, then `SdlPlatform::pump_events` (OS events → keys; focus loss releases all keys)
2. Frame-level actions: UI windows first (F7 panel, cushion menu, pause menu; since 15e Esc/Tab open the pause menu and its 시스템 tab quits), F3 debug toggle. `InputGate` decides whether the field gets the real input or none
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
- `entities` (optional, up to 256, phase 9c, spec 7.1): `{ "type": "training_dummy", "id": "meadow_dummy_1", "x": 10, "y": 14, "data": "props/training_dummy.json" }`. `type` must be known (`training_dummy`, or `monster` since 11a), `id` 1–64 bytes and unique in the map, `x`/`y` inside the map, `data` a content path. Two entities may not share a tile, and an entity may not stand on a spawn point or inside an exit area. Every entity must stand on a walkable tile; blocking types (`world::blocks_movement`, the dummy) then make it **blocked**. A monster does not block: its tile is its home for wandering. Since 13b `{ "type": "save_point", "id": "meadow_cushion", "x": 5, "y": 10, "display_name": "초원 입구" }` places a cat-nip cushion: no `data` (its look is the shared `props/catnip_cushion.bmp`), optional `display_name` 1–64 bytes (empty = the map's `display_name`), walkable, and its `id` may not equal a spawn id of the same map (cushion ids are used as spawn names). Cushion ids must be unique across the whole game (checked over `content/maps` by `MapLoader.RepositoryCushionIdsAreUniqueAcrossMaps`). Parsed by `wind/content/entity_loader`; the after-load checks (walkability, spawn/entity/exit placement) live in `wind/content/map_checks`. The `data` file is loaded with the field (`field_assets`); a broken dummy file rejects the map, so a blocked tile never appears without its dummy; a broken monster file only drops that monster.
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

Each direction is a row, each animation frame a column. `origin` is the feet point inside a frame. Frame durations are 1–10000 ms. The player sheet must provide `idle` and `walk`. A clip may set `"row"` (0–63, 13b) to always use that row whatever the facing (`SpriteSheet::frame_rect(clip, dir, column)`); the hero's optional `sit`, `curl` and `stretch` cushion clips use row 4 (front view). Missing optional clips fall back to `idle`.

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
| `lunge` | no (0) | 0–256 px the attacker's body travels during `active`, evenly, sliding along walls (phase 11b, monster leaps). The hitbox moves with the body |
| `projectile` | no | flying attack, same object as a spell's `projectile` (below). The `hitbox` is still required for now |
| `fx`, `sfx` | no | content paths (checked for safety; the files are loaded by presentation later) |
| `next_combo` | no | id of the next attack; must match the weapon's list order (below) |

**Hitbox frame.** The box is written for a hero facing **right**: its centre is `forward` px ahead of the feet and `lift` px lower on screen (negative = up, to reach body height); `length` runs along the facing, `width` across it. Facing left mirrors `forward`; facing up/down puts `forward` on the vertical axis and swaps `length`/`width`, while `lift` always stays a screen-vertical shift. `game/combat/place_hitbox` does this placement. Parsed by `wind/content/attack_loader`.

### Weapon (`content/weapons/*.json`, phase 9, spec 8.4)

```json
{ "schema_version": 1, "id": "longsword", "name": "롱소드", "weight": 6, "base_power": 14,
  "strength_scaling": 0.8, "stamina_per_attack": 14,
  "attacks": ["attacks/longsword_1.json", "attacks/longsword_2.json", "attacks/longsword_3.json"] }
```

- `name` 1–64 bytes (UTF-8), `weight` 0–100, `base_power` 0–999, `strength_scaling` 0–5, `stamina_per_attack` 0–200, optional `crit_bonus` 0–1, optional `attack_move_scale` 0–1 (walk speed multiplier while swinging, default 0 = stop; the dagger uses 0.5, phase 15a).
- Weapons shipped: `dagger` (4 hits), `longsword` (3), `greatsword` (2). Each weapon item in the catalog points at one; `game::WeaponRack` loads them all at start-up so equipping never reads files mid-game.
- `attacks`: 1–8 content paths in combo order. Every attack file is loaded and validated with the weapon; one broken file rejects the whole weapon and its own errors are reported too. Attack ids must be unique in the combo.
- `next_combo`, when present on an attack, must equal the id of the next attack in the list; the last attack must not have one. The list order is the source of truth.
- Parsed by `wind/content/weapon_loader` (`parse_weapon` takes an attack source, so tests run without files).

### Spell (`content/spells/*.json`, phase 9d, spec 8.6)

```json
{ "schema_version": 1, "id": "star_shard", "name": "별빛 조각",
  "intelligence_required": 10, "mp_cost": 6, "cast_seconds": 0.25, "power": 1.0,
  "projectile": { "speed": 360, "range": 240, "size": [6, 6] },
  "light": { "radius": 36, "color": "#a8dcef", "intensity": 0.8 } }
```

| Field | Required | Range / meaning |
|---|---|---|
| `id` / `name` | yes | 1–64 chars / 1–64 bytes UTF-8 |
| `intelligence_required` | yes | integer 1–60 |
| `mp_cost` **or** `mp_per_second` | exactly one | integer 1–999 per cast / 0.1–200 per second. `mp_per_second` only on a `beam`, and a `beam` must use it |
| `cast_seconds` | yes | 0–5 s before the spell goes off |
| `power` | yes | 0–10; damage = magic attack (spec 8.2) × power, then spec 9.3 |
| `projectile` **or** `melee` **or** `beam` | exactly one | the spell's motion (below) |
| `knockback`, `hitstop`, `shake` | no (0) | 0–1000 px/s, 0–0.5 s, 0–1; same meaning as on an attack (phase 10a) |
| `fx`, `sfx` | no | content paths |
| `light` | no | `radius` 1–512 px, `color` `#rrggbb[aa]`, `intensity` 0–1 (presentation only). Carried by each shard and by the casting hand glow, so switching spells never recolours a shard already in flight (15b) |
| `icon` | no (0) | integer 0–255: cell in `ui/spell_icons.bmp` for the HUD spell slot (15b) |
| `cast_move_scale` | no (0.4) | 0–1: walk speed multiplier while casting; 0 roots the caster (great star shard, 15c) |

- **File name = id (15b):** spells are found by id as `spells/<id>.json` (`game::SpellShelf`); a file whose `id` differs is rejected. Shipped: `star_shard`, `great_star_shard`, `star_swarm`, `moon_blade`, `comet_beam` (spec 8.6 table).
- **`projectile`** (shared with attacks): `speed` 1–2000 px/s, `range` 1–2000 px, `size` `[w, h]` 1–64 px; optional `count` integer 1–16 (default 1), `homing` 0–1 (default 0), `orbit_seconds` 0–10 (default 0). Until 15c only one straight shard flies; `count`, `homing` and `orbit_seconds` are validated now and move in 15c (star swarm).
- **`melee`**: same object as an attack `hitbox` (`offset`, `size`).
- **`beam`**: `length` 1–640 px, `width` 1–128 px, `tick_seconds` 0.02–1. Moves since 15d (`game::HeroBeam`): `power` is per second (one strike uses `power × tick_seconds`), `knockback`/`hitstop`/`shake` apply to every strike, `mp_per_second` is paid every step while held, and starting needs `combat::start_mp` = `mp_per_second × tick_seconds`.
- Any bad field rejects the whole spell. Parsed by `wind/content/spell_loader`; `hitbox` and `projectile` objects are read by the shared helpers in `wind/content/combat_readers`.

### Training dummy (`content/props/*.json`, phase 9c)

```json
{ "schema_version": 1, "id": "training_dummy", "name": "훈련용 허수아비",
  "max_hp": 100, "defense": 0, "recover_seconds": 1.5,
  "hurtbox": { "offset": [-10, -34], "size": [20, 34] },
  "sprite": { "image": "props/training_dummy.bmp", "frame": [48, 48], "origin": [24, 44] } }
```

- `max_hp` 1–99,999, `defense` 0–9,999, `recover_seconds` 0.1–30. `hurtbox.offset` is the top-left corner relative to the feet (−128…128 px), `size` 1–128 px. `sprite.image` a content path; `frame` 1–128 px; `origin` (the feet in the frame) must lie inside the frame. Frame 0 = standing, frame 1 = knocked down, side by side.
- Parsed by `wind/content/training_dummy_loader`. Feet are placed with `engine::tile_feet` (tile centre, ¾ down), the same rule as hero spawns.
- `hurtbox` and `sprite` are read by the shared helpers in `wind/content/body_readers` (also used by monsters). Their values are `combat::HurtboxShape` and `combat::SpriteStrip`.

### Monster (`content/monsters/*.json`, phases 11a–11d, spec 9.6 / 14.2)

```json
{ "schema_version": 1, "id": "slime_green", "name": "초록 슬라임",
  "max_hp": 30, "attack_power": 4, "defense": 0, "rune_reward": 12, "move_speed": 40,
  "detect_range": 120, "attack_range": 40, "leash_range": 260, "notice_seconds": 0.4,
  "wander": { "radius": 40, "speed": 18, "pause": [1.0, 2.5] },
  "body": [12, 6],
  "hurtbox": { "offset": [-9, -13], "size": [18, 13] },
  "sprite": { "image": "monsters/slime_green.bmp", "frame": [24, 24], "origin": [12, 21] },
  "attacks": ["attacks/slime_hop.json"], "stagger_seconds": 0.25, "death_fx": "fx/slime_burst.json" }
```

- `max_hp` 1–99,999; `attack_power`, `defense` 0–9,999; `move_speed` 1–400 px/s; `detect_range`, `attack_range`, `leash_range` 1–2,000 px with **attack ≤ detect ≤ leash**.
- `rune_reward` 0–9,999,999 (required, 13a): runes the hero gets when the monster falls (once per monster, on the step its HP reaches 0).
- `drops` (optional, 14b): up to 8 lines `{ "item": "slime_jelly", "chance": 0.5, "count": [1, 2] }`. `item` is a catalog id (checked against `items/catalog.json` when a field opens, and by the `MonsterDrops` test); `chance` above 0 and at most 1; `count` a whole number or `[min, max]` from 1 to 99 (default 1). Read by `wind/content/drop_table_reader`.
- `keep_away` 0–2,000 px (optional, 13c): if the hero is closer, the monster steps away (`Monster::step_back`) instead of attacking; must be smaller than `attack_range`. A blocked retreat attacks anyway. Used by the frog.
- An attack with `projectile` (13c) is a ranged monster attack: the windup warning runs the length of `projectile.range`, there is no contact hitbox, and when the attack starts the monster fires one shot (`Monster::take_shot` → `game::EnemyShots`). Shots fly like the star shard (`Projectile`), hit the hero's hurtbox unless she is invulnerable (dodge, hit, resting) and use the same damage path as lunges (`game::strike_hero`). Monster files: `slime_green`, `bat`, `mushroom`, `frog` (13c).
- `notice_seconds` 0–2 (the "!" pause before chasing).
- `wander.radius` 0–512 px and **`wander.radius + detect_range ≤ leash_range`** (a hero noticed from the edge of the wander area is still inside the leash); `wander.speed` 1–400 px/s; `wander.pause` `[min, max]` seconds, 0–30, min ≤ max.
- `body` `[w, h]` 1–64 px: the feet box for wall collision. `hurtbox` and `sprite` as for the training dummy. Sprite frame 0 = normal, frame 1 = attack windup (crouch, used from 11b).
- `attacks`: 1–6 attack file paths (same format as "Attack"; every file must load). 11b uses the first one. Monster attacks use `lunge` and set `cancel_window` to `[0, 0]` (no cancelling).
- `stagger_seconds` 0–2: how long a hit staggers the monster (and how long its knockback lasts, 11d). `death_fx` (optional): a content path to a particle effect burst where the monster vanishes; a broken effect file only removes the effect.
- Parsed by `wind/content/monster_loader` (attack files through the shared `AttackSource` in `attack_loader.hpp`, also used by weapons). Field still to come: `drop_table` (14).
- A broken monster file only removes that monster (logged once per file); the map still opens, because monsters do not block tiles.

### Progression (`content/progression/*.json`, phase 13a, spec 8.2 / 8.3)

`stat_curves.json`: one curve per derived value, each a list of `[stat, value]` points joined by straight lines.

```json
{ "schema_version": 1,
  "max_hp": [[10, 100], [20, 190], [30, 280], [40, 360], [60, 450]],
  "max_mp": [...], "max_stamina": [...], "carry_weight": [...],
  "strength_bonus": [...], "intelligence_bonus": [...], "spell_slots": [...] }
```

- All seven curves are required. 1–20 points each; stat is an integer 1–60, strictly increasing; value 0–99,999.
- Below the first point the curve stays at the first value; above the last point it stays at the last value.
- Which stat drives which curve is fixed in code (`derive_stats`, spec 8.2 table): vigor → `max_hp`, mind → `max_mp`, endurance → `max_stamina`, strength → `carry_weight` and `strength_bonus`, intelligence → `intelligence_bonus` and `spell_slots`. `max_hp`, `max_mp`, `max_stamina` and `spell_slots` are floored to whole numbers.

`levels.json`: level-up cost coefficients, cost from level L to L+1 = `floor(a + b·(L−1) + c·(L−1)²)`.

```json
{ "schema_version": 1, "a": 100, "b": 40, "c": 6 }
```

- `a` 1–1,000,000 (every level costs at least 1 rune), `b` 0–100,000, `c` 0–10,000. Levels run 1–251, so 250 → 251 is the last cost.
- Both files are parsed by `wind/content/progression_loader` (`load_progression_rules`). The client refuses to start if either file is broken, because max HP/MP/stamina come from them.
- Rules live in `shared/…/wind/progression/`: `Stats` (five stats, level = sum − 49), `StatCurve`, `derive_stats` (the one place derived values are computed), `level_up_cost`/`cost_between`, `RuneWallet` (0–999,999,999, all-or-nothing spending, `lose_half`). `game::HeroProgress` (client) owns stats + wallet outside the field, so they survive map travel and respawn; `try_raise` is all-or-nothing.

### Item catalog (`content/items/catalog.json`, phase 14a, spec 10.1)

One file lists every item. The order in the file is the order the inventory sorts by.

```json
{ "schema_version": 1,
  "items": [
    { "id": "hp_potion", "name": "회복 물약", "description": "HP를 50 채운다.",
      "kind": "consumable", "max_stack": 20, "icon": 0, "rarity": "common",
      "use": { "heal_hp": 50 } },
    { "id": "slime_jelly", "name": "슬라임 젤리", "description": "...",
      "kind": "material", "max_stack": 99, "icon": 2 },
    { "id": "longsword", "name": "롱소드", "description": "...",
      "kind": "weapon", "max_stack": 1, "icon": 6, "rarity": "rare",
      "weapon": "weapons/longsword.json" } ] }
```

- 1–256 items, ids unique (1–64 bytes). `name` up to 64 bytes, `description` up to 256 bytes (UTF-8).
- `kind`: `consumable`, `material` or `weapon`. `rarity` (optional, default `common`): `common`, `uncommon`, `rare`.
- `max_stack` 1–999, `icon` 0–255 (cell index in the item icon sheet, phase 14c).
- `use` is required for `consumable` and not allowed otherwise: `heal_hp`, `heal_mp` (each optional, 0–99,999; at least one above 0).
- `weapon` is required for `weapon` and not allowed otherwise: a content path that must load as a weapon file (attacks included). Weapon items must have `max_stack` 1; their equip slot is `weapon` (equipping comes in phase 15).
- Parsed by `wind/content/item_catalog_loader` (`load_item_catalog`) into `items::ItemCatalog` (find by id, `rank` = file order).
- `items::Inventory` (shared, pure rules): 24 slots by default, stacks fill before new slots, `try_add`/`try_remove` are all-or-nothing, `take_at` takes from one slot, `sort` merges stacks in catalog order and keeps unknown ids at the back.
- Phase 14b runtime: `client/src/app/item_setup` loads the catalog at start (a broken catalog stops the game, like maps) and owns the bag and `game::LootStore` (ground items per map file, outside the field so they survive travel). The field borrows them as `game::ItemWorld`. `FieldItems` runs `FieldLoot` (rules: drops with its own dice, pickup) in the fixed step and `LootView` (presentation) per frame; pickups and bag-full refusals become `LootNotice`s that the app shows with `game::LootToasts`.
- Item icons: `items/item_icons.bmp`, 16×16 cells in a row, cell = `icon`.

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
- `InputBindings` maps game `Action`s to keys. Defaults: arrows/WASD move, Shift run (held), Space/K/X dodge, J/Z attack, L/C cast, E interact, I menu on the bag tab (14c, 15e), R sort bag (14c), Q next spell (15b), Enter confirm (UI), F3 debug overlay, F4 collision overlay, F5 skip time, F6 next weather, F7 UI test panel, F12 screenshot. Since 15e: `Back` = Esc (UI back/close), `Menu` = Esc or Tab (open the pause menu), `NextTab` = Tab (next menu tab); Esc no longer quits (`Quit` became `Back`). `Key::Tab` is new (SDL, `--press tab`). Left and right Shift both map to `Key::Shift`.
- `MovementIntent` turns held move keys into an 8-way unit vector (diagonals are normalized). When opposite keys are both held, the most recently pressed one wins.
- `InputBuffer` remembers a dodge, attack or cast press for **0.15 s**, so a press slightly before a dodge becomes possible still fires. Presses are recorded per frame and consumed in fixed steps, so a press is never lost on a frame that runs zero steps.
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
- Draw order after lighting: world `Glow…DebugOverlay` → damage numbers → grayscale (12b, only while the hero is down) → HUD + banner + fallen screen → transition cover → travel notice → UI test panel → F3 panel → screenshot flash.
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

- `game::ExitWatcher` (13b, moved out of `FieldState`) checks the feet tile after every simulation step. An exit fires only after the hero has stood on a non-exit tile once ("armed"), so arriving next to or on an exit never bounces her straight back. A fallen or resting hero never leaves.
- `game::MapTravel` (SDL-free, `game/world/map_travel`) runs the sequence. `ClientApp` passes it the field's exit request; it covers the screen with `ScreenFade` (0.35 s, `in_quad`) in the exit's `transition` style, calls the `EnterMap` callback once when fully covered, then uncovers (0.35 s, `out_quad`). Input is ignored while busy. The F3 panel stays visible on top.
- `EnterMap` builds the new `FieldState` for `to_map` / `to_spawn` and swaps it in only on success. On failure the current field is kept, the reason is logged, and `MapTravel::notice()` holds a short English message for 4 s that `ClientApp` draws in red at the bottom of the screen (debug font until the phase 8 text renderer). Tested in `map_travel_tests.cpp` and `field_assets_tests.cpp`.
- **Fade** draws one translucent black rectangle. **Iris** (`engine/fx/iris_mask`) leaves a round hole centred on the hero's body (`FieldState::focus_on_screen`) and covers the rest with opaque black: the hole radius goes from the distance to the farthest screen corner down to 0, and the cover is built as one rectangle per pixel row on each side of the hole (plus one block above and below), so the edge is pixel-exact. When opening, the hole is centred on the hero in the new map.
- Each `FieldState` destroys the textures it generated (shadow, particles, light) when it is replaced.
- Respawn (12c) uses the same path: `MapTravel::begin(exit, TravelOptions{TravelKind::Respawn, 0.8})`. Since 13b the target comes from `game::respawn_target`: the last cushion rested at (`HeroProgress::last_cushion`, possibly another map), else the current map's `default` spawn. `ScreenFade::fade_out(style, seconds)` takes a per-trip length (0 = default) that the following `fade_in` reuses. `EnterMap` receives the `TravelKind` (Exit, Respawn, Rest); on a normal exit it copies `HeroVitals` from the old field into the new one (`FieldState::hero_vitals` / `restore_hero`), on a respawn the new field starts full. `game::RespawnTrigger` (SDL-free) decides when: Enter, or 2.5 s after the fallen screen is ready.
- Resting (13b) uses the same path too: `FieldState::take_rest_request` (0.6 s after sitting) → `TravelKind::Rest`, 0.5 s fade, the **same** map is read again with the cushion id as spawn (`load_field_assets` looks up spawn points first, then cushions, then falls back to the first spawn for `default`), so monsters are back and the hero starts sitting on the cushion (`FieldAssets::start_cushion` → `FieldRest::sit_on`). Then `HeroProgress::rest_at` sets the respawn cushion; on a first discovery the cushion blooms and the banner shows "캣닢 쿠션을 찾았다 — 이름". The autosave step only logs until phase 16.
- The travel plumbing (field, `MapTravel`, `RespawnTrigger`, current map, enter callback) lives in `client::FieldSession` (`app/field_session`), split out of `ClientApp` in 13b.
- After a rest trip (13c) `FieldSession::take_menu_request` asks the app to open the cushion menu once the fade is over (`CushionMenuPanel`: keys → `game::CushionMenu` → field). While it is open the app still advances the fixed-step clock but runs no steps, so the simulation and game clock pause while frame updates (wind, particles) go on.
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

F3 toggles: FPS and average frame time (0.5 s window), dropped-time frames, map id, feet position (px) and tile, movement state (idle/walking/running/dodging), facing, speed, stamina (and "exhausted"), draw calls, culled sprites, tiles queued, particle count and drops, texture count, cache hits, load failures, UI text draws / cached texts / clipped texts this frame, and two lines of key help. F4 toggles the red collision overlay, the cyan feet box, the yellow hurtboxes of standing dummies and the red sword hitbox while it is active. Phase 9c adds a combat line, extended in 9e: `mp 44/50  dummies N  shards N  last hit A -> #id hp/max (down)`; F4 also shows flying shards as purple boxes. Scripted runs can press `l` (`--press l@60`). Phase 10a marks critical hits with `!` (`last hit 21!`). Options `--shake <0-1>` and `--flash <0-1>` (phase 10b) scale screen shake and hit flash; they are kept in `ClientOptions::feel` and applied to every new field. Since phase 8 the panel starts **hidden**, because the HUD now sits in the top-left corner (`--press f3@2` shows it in scripted runs).

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
- Shards are stopped by every non-walkable tile, water included (there is no "blocks shots" tile flag yet).
