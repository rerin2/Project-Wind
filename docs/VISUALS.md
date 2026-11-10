# Visual Rules and Effect Tuning

Status: phase 6 (time of day and weather). Spec reference: master spec section 6.

Effects are presentation only: they read simulation state and events and never change gameplay results.

## Palette (draft)

A shared 32-colour palette for all art (spec 6.1). The current test art predates it and will be recoloured when real art arrives (phase 20). Regions pick their mood from the same palette: meadow = warm greens, forest = deep greens, cave = cool blues and purples.

| # | Name | Hex | Use |
|---|---|---|---|
| 0 | ink | `#14101c` | outlines, deepest shadow |
| 1 | night | `#232142` | night ambient, cave darkness |
| 2 | dusk | `#3b3366` | deep shadow, cave walls |
| 3 | violet | `#5d4f8f` | magic, cave highlights |
| 4 | lavender | `#9a8fc7` | magic glow, distant sky |
| 5 | deep sea | `#1d3a5c` | deep water |
| 6 | sea | `#2f6690` | water |
| 7 | sky | `#5fa3d0` | water highlights, sky |
| 8 | ice | `#a8dcef` | sparkle, snow shadow |
| 9 | snow | `#f2f6f7` | snow, brightest highlight |
| 10 | pine | `#1e3d2f` | forest shadow |
| 11 | forest | `#2e6b3f` | dark grass, leaves |
| 12 | grass | `#4c9a46` | grass |
| 13 | leaf | `#7cc45a` | grass highlight |
| 14 | lime | `#c3e57a` | sunlit leaves, fireflies |
| 15 | bark | `#3f2a24` | tree trunks, dark wood |
| 16 | wood | `#6b4532` | wood, dirt shadow |
| 17 | dirt | `#a06b47` | paths |
| 18 | sand | `#d4a86e` | sand, dust |
| 19 | wheat | `#ecd59f` | light dust, paper |
| 20 | blood | `#6e1f2c` | damage shadow |
| 21 | red | `#b83a3a` | damage, danger warnings |
| 22 | ember | `#e86a3a` | fire, critical hits |
| 23 | gold | `#f2b544` | torchlight, coins, level-up |
| 24 | sun | `#fff0a8` | light cores, sparks |
| 25 | rose | `#d97a9b` | flowers, dawn tint |
| 26 | blush | `#f2b8c6` | petals, dawn highlight |
| 27 | stone dark | `#4a4a58` | stone shadow |
| 28 | stone | `#7a7a8a` | rocks, walls |
| 29 | stone light | `#b0b0bd` | stone highlight |
| 30 | skin | `#e8b896` | skin |
| 31 | skin shadow | `#b9806a` | skin shadow |

Rules: characters must stand out from the ground (outline in `ink`, see spec 6.1); light effects use `gold`/`sun`/`ember` with additive blending; darkness uses `night`/`dusk` with multiply blending.

## Hero

Test art drawn by `tools/test_content/generate_test_content.ps1` (32 × 48 frames, feet at y = 46).

| Part | Look |
|---|---|
| Hair | silver, long and straight to the waist, bangs, side locks to the shoulders; base `#ced4e4`, shade `#9ca2be`, shine `#f2f4fc` |
| Cat ears | silver outside, pink inside `#eea8ba` (back view: no pink) |
| Eyes | violet `#7a60d6` with a white shine pixel, dark lash line, light blush |
| Outfit | navy dress `#3c4680` with white collar and hem, red ribbon, dark tights, brown boots |
| Outline | 1 px `ink` (`#1c1628`) around the whole figure, so it reads on grass |
| Walk | legs alternate, body bobs 1 px, hair tips sway 1 px the other way |

The right-facing row is a mirror of the left-facing row.

## Blend modes

`SpriteDraw::blend` (spec 5.2):

| Mode | SDL mode | Result | Use |
|---|---|---|---|
| `Normal` | `BLEND` | sticker over the picture; transparent parts show through | sprites, UI |
| `Add` | `ADD` | adds colour → brighter; black has no effect | lights, sparks, fire |
| `Multiply` | `MUL` | multiplies colour → darker; white has no effect, transparent parts leave the picture alone | shadows, night darkness |

`SpriteDraw::tint` multiplies the texture colour (white = unchanged) and its `a` is the opacity.

## Screenshot feedback

Code: `client/src/app/screenshot_flash.cpp`. The screenshot is saved first; the feedback starts on the next frame so it never appears in the picture.

| Setting | Value |
|---|---|
| White flash | peak alpha 170, fades over 0.30 s (`out_quad`) |
| Label | "screenshot saved", bottom-right, fades over 1.2 s (`in_cubic`) |

## Character shadow

| Setting | Value |
|---|---|
| Shape | soft ellipse, generated in code (`Renderer::create_soft_ellipse_texture`) |
| Size | 22 × 8 px, centred on the feet |
| Colour | black, centre alpha 110 (≈ 43 %) |
| Layer | `Shadows` (above ground decor, below characters) |

## Particles

Effect files live in `content/fx/` (format: [ENGINE.md](ENGINE.md#particle-effect-contentfxjson)). Change the look there, not in code.

| Effect | File | Look | Key values |
|---|---|---|---|
| Walking dust | `dust_walk.json` | soft tan puff behind the feet | 1 puff every 14 px walked (above 40 px/s, rule in `dust_emitter.cpp`); 0.35–0.55 s; 4–6 px → 1 px; `#d6c4a0` alpha 170 → 0; drag 5, drifts up |
| Dodge dust | `dust_dodge.json` | burst of puffs thrown backward | 10 puffs; spread 140°; 20–60 px/s; alpha 200 → 0 |
| Petals | `petals.json` | pink pixel petals drifting down-left, swaying | 6 per second around the view; 6–9 s; 3–4 px; `#f590b0` → `#fbd0dc`; sway 7 px at 0.5 Hz; fade in 8 %, out 20 % |
| Fireflies | `fireflies.json` | soft yellow-green glows wandering slowly | additive blend; 2.5 per second; 3–5 s; 5–7 px; `#fff6a0` → `#d8f080`; fade in 30 %, out 35 % (stronger at night in phase 6) |

Pools: ground (dust) 1024, air (ambience) 4000. Extra particles are dropped and counted on the F3 panel.

Lesson from tuning: fading the colour over the whole lifetime made petals half-transparent most of the time and hard to see; `fade_out` keeps them solid until the end.

## Lighting

How it works: [ENGINE.md](ENGINE.md#lighting-spec-63-phase-5). Values used in the test cave (`content/maps/test_cave.json`):

| Light | Colour | Radius | Intensity | Animation |
|---|---|---|---|---|
| Ambient (cave darkness) | `#1c1a30` (palette `night`-ish) | — | — | — |
| Torch | `#ffb060` (warm `gold`) | 110 px | 1.0 | flicker 0.35 + `torch_embers` sparks |
| Pool magic light | `#5878ff` (blue) | 140 px | 0.9 | pulse 0.4 Hz, depth 0.5 |
| Hero light | `#ffe2b8` (warm white) | 80 px | 0.55 | follows the hero, 14 px above the feet |

| Effect | File | Look |
|---|---|---|
| Torch embers | `fx/torch_embers.json` | 1–2 px orange sparks rising and swaying, additive, 6 per second per torch |

Tile 11 `torch`: wooden pole, iron cup, three-tone flame (`ember`, `gold`, `sun`). Blocks movement.

Tile 12 `cave_entrance`: dark arch in a stone wall, walkable; used as the exit tile on both sides (meadow left wall at (0, 12), cave left wall at (0, 10)).

Map travel fade: black, 0.35 s out (`in_quad`) and 0.35 s in (`out_quad`).

Rules of thumb: keep ambient dark but not black (players must still read walls); torch pools should overlap a little so paths stay readable; avoid many overlapping lights at full intensity (they add up to white).

## Time of day

File: `content/time/day_cycle.json`. One game hour = 60 real seconds.

| Hour | Ambient | Night | Mood |
|---|---|---|---|
| 0:00–4:30 | `#2a3060` → `#30366a` | 1 | deep blue night |
| 6:00 | `#c890a8` | 0.5 | pink dawn |
| 7:30 | `#f4e8e0` | 0 | soft morning |
| 12:00 | `#ffffff` | 0 | full daylight (no change to the art) |
| 16:30 | `#fff2e0` | 0 | warm afternoon |
| 18:30 | `#f0a070` | 0.3 | orange sunset; lanterns start to glow |
| 19:30 | `#906c9c` | 0.7 | purple dusk |
| 21:00 | `#2a3060` | 1 | night |

Night-only things: lanterns (`night_only` lights, `#ffd890`, radius 100) and fireflies (`night_fx`). Fireflies and torch embers are additive and drawn after lighting (`Glow` layer) so they stay bright in the dark.

Tile 13 `lantern`: wooden post with a glass lamp; four stand along the meadow path at (10, 17, 26, 34; row 11).

## Weather

File: `content/weather/meadow.json`, 5 s transitions.

| State | Weight | Lasts | Look |
|---|---|---|---|
| clear | 4 | 90–180 s | nothing extra |
| rain | 2 | 45–90 s | `fx/rain.json`: 1 px streaks 10–15 px long, `#e6eeff` 82 % alpha, 360–440 px/s at 102°, 520 per second; `fx/rain_splash.json`: 1 px splashes, 160 per second; tint `#a0aabe`; light fog 0.15 |
| snow | 1 | 45–90 s | `fx/snow.json`: 2–3 px white pixel flakes, 14–26 px/s, sway 6 px at 0.35 Hz, 60 per second; tint `#dce2f0` |
| fog | 1 | 45–90 s | 7 drifting fog banks `#d6dce8`, alpha up to 84 (0.7 × 120); tint `#d0d4dc` |

Lessons from tuning: pale 1 px streaks and soft 2–4 px snow dots vanish on bright grass. Rain needed brighter, longer, denser streaks plus a darker tint; snow needed crisp pixel flakes. Effect `rate` was raised to allow up to 2000 per second for heavy rain.

Forest table (`content/weather/forest.json`, 5 s transitions): clear 3 (90–180 s), fog 3 (60–120 s, tint `#c8d0cc`, fog 0.8), rain 2 (45–90 s, same effects as the meadow, tint `#98a4b4`, fog 0.2). The forest leans foggy. Its ambient effect is `fx/leaves.json` (green-to-amber pixel leaves, 5 per second, slow wobbling fall).

## Living tiles and wind (phase 7)

Data: `content/tilesets/test_tiles.json`. Code: `client/src/engine/map/tile_animation.cpp`.

| Tile | Effect | Setting | Why |
|---|---|---|---|
| 4 water | frames 4 → 14 → 15 → 14 | 0.35 s per frame | Ripple lines slide 2 px per frame and 1–2 small highlights appear; ping-pong order avoids a visible jump back to the first frame |
| 9 flowers | frames 9 ↔ 16 | 0.7 s per frame | Only the flower heads lean 1 px; stems stay, so it reads as a breeze, not a jitter |
| 7 tree canopy (meadow) | sway | 1 px, 3.5 s period | Slow and small: leaves move, trunks stay planted |
| 21 dark canopy (forest) | sway | 1 px, 4.2 s period | Slightly slower than the meadow trees, heavier crowns |
| 17 tall grass (forest) | sway | 1 px, 2.4 s period | Faster than trees so the ground layer feels lighter |

- Neighbouring cells start on different animation frames (stagger 7·x + 13·y frames), so a pond never blinks in unison.
- The sway wave lags 0.45 rad per column and 0.2 rad per row, so gusts appear to roll left to right.
- Offsets are whole pixels (no blurry half-pixel draws). More than 1–2 px looks like the tiles are sliding; keep it small.

## Screen transitions (phase 7)

Code: `client/src/engine/fx/screen_fade.cpp`, `iris_mask.cpp`, drawn in `client/src/app/client_app.cpp`.

| Style | Used by | Look |
|---|---|---|
| fade (default) | meadow ↔ forest | full-screen black, alpha follows `in_quad` closing / `out_quad` opening, 0.35 s each |
| iris | meadow ↔ cave | opaque black outside a circle centred on the hero's body; radius from the farthest screen corner to 0 with the same easing and timing; opens on the hero in the new map |

The iris edge is built per pixel row and rounded to whole pixels, so it stays crisp at any integer window scale. Walking into a cave mouth reads as "going in"; open-air paths use the softer fade. A failed exit shows a red one-line notice on a dark band near the bottom for 4 s ("갈 수 없는 곳이에요: …", Galmuri11 since phase 8).

## HUD and text (phase 8)

Code: `client/src/app/field_ui.cpp`, `ui_test_panel.cpp`, `client/src/engine/ui/ui_painter.cpp`; look data in `content/ui/theme.json`; frames drawn by the test-content generator (`content/ui/ui_frames.bmp`, 72×24).

| Element | Look | Why |
|---|---|---|
| Font | Galmuri11 12 px regular/bold, mono hinting, 1 px shadow `#10131c` below | Designed for 12 px, so every glyph is a clean pixel shape at 640×360; the shadow keeps text readable on bright grass |
| Text colours | `#f2efe6` text, `#9aa3b5` dim, `#ffe9a8` focused/title | Warm off-white and gold sit with the palette instead of pure white |
| Panel | 24×24 9-slice, border 8: dark outline, silver edge `#96a0b8`, navy body `#1c2032`, gold corner dots, 92 % opacity | Readable over any map, a little of the world shows through |
| Buttons | 24×16, border 5; normal navy with grey edge, focused brighter with gold edge and gold text | Focus is clear without an extra cursor |
| Stamina bar | top-left (12, 12), 82×7 with 1 px dark outline; fill `#7ece5c`, trail `#faeec4`, empty `#282c3c` | Small and out of the way (spec 13.2); the cream trail shows how much was just spent |
| Stamina auto-hide | dims to 30 % after 1.2 s at full, back to full opacity on the first spend | Stays visible only when it matters |
| Exhausted | fill and outline `#de5c48`, blinking 4 Hz | Visible even when the bar is empty |
| Area banner | bold name centred at y = 44, two 36 px gold lines 8 px from the text, translucent black band behind; 0.5 s slide-in (8 px) and fade, 2.2 s hold, 0.7 s fade-out | Announces the place without covering the hero |
| UI test panel (F7) | 240×178, pops in over 0.2 s (`out_back` from 85 % size) | Spec 6.9: windows open with a 0.15–0.25 s tween |

HUD and banner are drawn after lighting (never darkened by night or caves) but under the transition cover, so they fade out and in with the map.

## Dodge afterimages

Code: `client/src/game/presentation/afterimage_trail.cpp`.

| Setting | Value |
|---|---|
| Snapshot interval | 0.035 s while dodging |
| Fade | 0.18 s, alpha 120 → 0 |
| Order | drawn just behind the live sprite |

## Camera

| Setting | Value |
|---|---|
| Follow stiffness | 8 (exponential) |
| Look-ahead | up to 28 px toward movement, sharpness 3, scaled by speed |
| Snapping | camera top-left and sprites rounded to whole pixels |

## Animation

- Walk animation speed scales with movement speed (minimum 0.5×), so feet do not slide at low speed.
- Dodge plays the walk clip at 2× until a dedicated roll animation exists.
