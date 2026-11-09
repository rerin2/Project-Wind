# Visual Rules and Effect Tuning

Status: phase 3 (rendering foundation). Spec reference: master spec section 6. Later phases add particles (4), lighting (5), and day/night and weather (6).

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

## Dust

Code: `client/src/game/presentation/dust_emitter.cpp`.

| Setting | Value |
|---|---|
| Texture | 8 × 8 soft circle, colour (214, 196, 160) |
| Walk puff | every 14 px walked, only above 40 px/s |
| Dodge burst | 10 puffs, pushed backward with a sideways spread |
| Lifetime | 0.35–0.55 s |
| Size | 4–6 px shrinking to 1 px |
| Alpha | 170 fading to 0 |
| Motion | drag 5, slight upward drift (gravity −10) |
| Pool | 512 particles; extra puffs are dropped and counted (F3) |

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
