# Visual Rules and Effect Tuning

Status: phase 2 (first effects). Spec reference: master spec section 6. Later phases add the palette (3), particles (4), lighting (5), and day/night and weather (6).

Effects are presentation only: they read simulation state and events and never change gameplay results.

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
