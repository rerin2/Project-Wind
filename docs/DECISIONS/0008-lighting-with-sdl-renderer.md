# 0008 — Lighting stays on the SDL renderer (no shader pipeline yet)

- Status: accepted
- Date: 2026-11-09
- Spec reference: master spec 2.1, 6.3, 23.2 ("SDL renderer vs SDL GPU")

## Decision

Lighting uses the existing `SDL_Renderer` with render targets and blend modes. We do not switch to the SDL3 GPU API (custom shaders) now.

How it works:

1. The world is drawn into the 640×360 frame target (ADR 0006, phase 3).
2. A second 640×360 **light map** target is cleared to the map's ambient colour.
3. Each visible light is drawn into the light map as a white radial gradient (`(1 - d²)²` falloff), tinted with the light colour, opacity = strength, blend `ADD`.
4. The light map is drawn over the frame target with blend `MOD` (`dst = src × dst`), so white keeps the picture and dark colours darken it.
5. Foreground and debug layers are drawn after lighting, so debug overlays are never darkened.

Maps without `ambience.ambient_light` skip the pass entirely.

## Evidence (phase 5, dev machine, Debug build, VSync on)

| Scene | Lights | Result |
|---|---|---|
| Test cave | 10 (8 drawn) | 60.0 FPS, 16.64 ms/frame |
| Test cave + `--light-stress 32` | 42 (37 drawn) | 60.0 FPS, 16.67 ms/frame, 302 draw calls |

The look matches spec 6.3 (dark ambient, warm flickering torch pools, a pulsing blue light, a small light that follows the hero).

## Limits we accept

- Lights are not blocked by walls (no shadows or occlusion). Light passes through walls.
- Overlapping lights add up and can saturate to near-white; keep intensities moderate in content.
- No bloom or glow outside the light radius. Emissive effects (fireflies at night) need their own small light or must be drawn after the light pass.
- The light texture uses linear filtering (it is scaled up a lot); everything else stays nearest-neighbour.

## When to revisit

Switch to (or add) the SDL3 GPU API if a later phase needs any of: shadows/occlusion, bloom, colour grading beyond multiply, distortion (heat haze, water), or if the light pass costs more than ~2 ms per frame on target hardware.

## Alternatives considered

- **SDL3 GPU API with shaders now** — most flexible, but a large new pipeline (shader compilation per backend) before we know we need it.
- **Per-tile lighting (darken tiles by distance)** — cheap, but blocky and does not light characters or particles smoothly.
