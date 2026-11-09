# Visual Rules and Effect Tuning

Status: phase 5 (lighting). Spec reference: master spec section 6. Phase 6 adds day/night and weather.

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
