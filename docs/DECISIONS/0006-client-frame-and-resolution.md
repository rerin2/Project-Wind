# 0006 — Client frame timing, resolution, tiles and directions

- Status: accepted
- Date: 2026-11-02
- Spec reference: master spec 5.1, 5.3, 5.4, 5.6, 25.2

## Decisions

| Item | Value | Where |
|---|---|---|
| Client game-update step | fixed 60 Hz | `client/src/app/client_app.cpp` |
| Max frame time counted | 0.25 s | same |
| Max fixed steps per frame | 5 (extra time dropped) | same |
| Logical resolution | 640×360, integer-scaled to the window | same |
| Tile size | 32×32 (per-map `tile_size`, must match tileset) | content |
| Directions | 4 (down, left, right, up) | `wind/world/direction.hpp` |
| Walk speed (phase 1, local) | 0.2 s per tile | `game/states/field_state.cpp` |

## Reasons

- A fixed update step keeps movement and animation independent of render FPS; the caps prevent a burst of updates after a stall (window drag, debugger break).
- 640×360 scales by exactly 2× to 1280×720 and 3× to 1920×1080 with no blurring; at 32 px tiles it shows 20×11 tiles.
- 32 px tiles and 4 directions are the spec's initial defaults (25.2). `Direction` and the sprite sheet's `direction_rows` keep 8-direction expansion possible.
- Walk speed is a placeholder until the `movement_rate` stat exists (phase 2/3), when the server will own it.

## Note

The client's 60 Hz step is presentation timing only. The server's authoritative tick stays at 20 Hz ([0004](0004-server-tick-rate.md)); the two must not be confused.
