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
| Bar order (phase 12a) | three rows at the top left, x = 12: HP (y = 12), MP (y = 21), stamina (y = 30); every bar is 7 px tall with a 1 px dark outline and a 2 px gap; inner length = max value × 0.8 px, clamped 8–200 and rounded to whole pixels (`game/ui/hud_bar_layout`) | Spec 13.2 order HP·MP·stamina; one pixels-per-point rule so the bars compare at a glance and grow when stats rise |
| HP bar (phase 12a) | top row, 100 HP → 80 px inner; fill `#d64840` (same red as enemy HP bars), trail `#faeec4`, empty `#282c3c`; never fades | Red is HP everywhere; the cream trail shows how much a hit just took. Always visible because it is the number that decides survival |
| Stamina bar | bottom row (y = 30), 100 stamina → 80 px inner; fill `#7ece5c`, trail `#faeec4`, empty `#282c3c` | Small and out of the way (spec 13.2); the cream trail shows how much was just spent. It moved from y = 12 to the bottom row in phase 12a |
| Stamina auto-hide | dims to 30 % after 1.2 s at full, back to full opacity on the first spend | Stays visible only when it matters |
| Exhausted | fill and outline `#de5c48`, blinking 4 Hz | Visible even when the bar is empty |
| MP bar (phase 9d) | middle row (y = 21), 50 MP → 40 px inner; fill `#5fa3d0` (palette sky), trail `#faeec4`, empty `#282c3c`; never fades | Blue is the usual MP colour and stays apart from green stamina and red HP |
| Not enough MP (phase 9e) | when a cast is refused, the MP bar outline blinks `#de5c48` at 8 Hz for 0.6 s (alpha 80–255) | Spec 9.2: the player must see why nothing happened; faster than the 4 Hz exhausted blink because it is a one-off "no" rather than a lasting state |
| Area banner | bold name centred at y = 44, two 36 px gold lines 8 px from the text, translucent black band behind; 0.5 s slide-in (8 px) and fade, 2.2 s hold, 0.7 s fade-out | Announces the place without covering the hero |
| UI test panel (F7) | 240×178, pops in over 0.2 s (`out_back` from 85 % size) | Spec 6.9: windows open with a 0.15–0.25 s tween |

HUD and banner are drawn after lighting (never darkened by night or caves) but under the transition cover, so they fade out and in with the map.

## Training dummy and slash arc (phase 9c)

Code: `client/src/game/presentation/combat_view.cpp`, `slash_arc.cpp`. Art from the test-content generator: `content/props/training_dummy.bmp` (96×48, two 48×48 frames, feet at 24,44) and `content/fx/slash_arc.bmp` (32×32 white crescent).

| Element | Look | Why |
|---|---|---|
| Dummy | wooden post and arm bar, straw sack body with rope belt, red-and-white target on the chest, burlap head with stitched face and straw tufts; same soft shadow as the hero (20×7) | Reads as "something to hit" at 32 px tiles; the target shows where the hurtbox is |
| Knockback sway (10a) | the sprite is drawn at feet + the simulated body offset (up to 8 px toward the hit), the shadow stays on the planted spot | The gap between body and shadow reads as "pushed" even at a few pixels |
| Knocked down | frame 1: the same dummy tilted 22° | Clear state change without new animation work; the frame is wide enough that the head is not clipped |
| HP bar | 24×3 inside a 1 px dark outline, 3 px above the head; fill `#d64840`, trail `#faeec4`, empty `#282c3c`; drawn after lighting | Red = enemy HP, cream trail = damage just dealt (same delayed bar as stamina) |
| HP bar visibility | hidden while full (also right after entering the map), shown on the first hit, hidden again 2.5 s after it has refilled | Standing dummies do not clutter the screen |
| Slash arc | white crescent tinted `#dcecff`, additive, drawn in the Glow layer at the hitbox centre, rotated to the facing (right 0°, down 90°, left 180°, up 270°), 6 px longer than the hitbox | No attack animation exists yet; the arc shows where and when the sword hits, and glows at night and in caves |
| Slash timing | appears when the hit's `active` part starts, sweeps 50° over the first 60 % of its life, fades over the rest; lives `active + 0.08 s`; even-numbered combo hits sweep the other way | A quick "whoosh" that matches the hit window; alternating sweeps make the combo read as back-and-forth slashes |
| F4 boxes | sword hitbox `#f03c3c` at 110 alpha (only while active), dummy hurtboxes `#f0d23c` at 100 alpha | Debug only |

Known look issue: when the hero stands directly above a dummy, its HP bar overlaps her waist. Acceptable for a training prop; revisit with enemy HP bars in phase 11.

## Star shard (phase 9e)

Code: `client/src/game/presentation/spell_view.cpp`; effects `content/fx/star_trail.json`, `content/fx/star_burst.json`; light from `content/spells/star_shard.json`. All drawing uses the soft round light texture with additive blend in the Glow layer, so it glows at night and in caves and never changes the result.

| Element | Look | Why |
|---|---|---|
| Cast glow | at the hand point (10 px ahead, 14 px up), grows from 4 to 14 px over the cast: halo `#a8dcef` at 1.8× size, white core `#f2f6f7` at 0.6×; adds a light of half the spell radius whose brightness follows the cast progress | No cast animation exists yet; the growing spark shows the 0.25 s wind-up and where the shot will come from |
| Shard | halo 20 px `#a8dcef` (alpha 210) twinkling ±12 % at 9 Hz, white core 7 px; drawn between the last two fixed steps | Palette ice/snow = "starlight"; the twinkle keeps a small 6 px shot readable at speed |
| Trail | `star_trail`: pixel sparks, 70/s, 0.25–0.45 s, drift 4–14 px/s, ice → violet `#5d4f8f` fading out | A short glittering tail shows direction and speed without hiding the target |
| Vanish | `star_burst`: 12 white → ice sparks, 30–75 px/s, 0.2–0.4 s, at the point the shard ended (wall, target or range) | Tells the player the shot is gone and where; damage feedback (flash, numbers) is in "Hit feel" below |
| Light | radius 36, `#a8dcef`, intensity 0.8, one per shard (added after the hero light so it is never cut by the light cap) | Spec 8.6: spells light their surroundings; at night the shot lights the grass as it flies |
| F4 box | shard hit rect `#aa78ff` at 140 alpha | Debug only; purple keeps it apart from sword (red) and hurtboxes (yellow) |

## Hit feel (phase 10b)

Code: `client/src/game/presentation/field_combat_fx.cpp` (gathers hits per fixed step, plays them per frame), `screen_shake.cpp`, `damage_numbers.cpp`, `combat_view.cpp` (flash), `client/src/app/damage_number_drawing.cpp`; effect `content/fx/hit_spark.json`. Hitstop and knockback are simulation (phase 10a, `docs/GAMEPLAY.md`). All of this only reads hit events; none of it changes the result.

| Element | Look | Why |
|---|---|---|
| White flash | the hit dummy is drawn a second time with additive blend, white tint, alpha 255 → 0 over **0.08 s** (spec 6.7), times the flash setting | Reads as a bright pop on the frame of impact. It brightens toward white rather than a flat white silhouette, which would need a mask texture; good enough for straw and wood |
| Screen shake | trauma += attack `shake` × 2 (max 1), decays 1.6/s (full → 0 in about 0.6 s); offset = 6 px × setting × trauma², from two sine waves (11 Hz and 17 Hz, y phase-shifted); applied to a copy of the camera used for drawing only | Spec 6.7 "intensity² ": single light hits barely move the view (sword hit 1 ≈ 0.5 px), the finisher (0.35 → trauma 0.7 ≈ 3 px) and fast combos clearly do; sines keep it smooth and testable instead of jittery noise. The follow camera itself never shakes, so it never drifts |
| Hit sparks | `hit_spark`: 9 streaks, gold-white `#fff0a8` → `#f2b544`, 70–150 px/s within 80° of the push direction, 0.12–0.25 s, additive; critical hits burst twice | Points the eye along the hit direction; one shared effect until more weapons arrive (phase 15, per-attack `fx`) |
| Damage number | starts 32 px above the hurtbox centre (just above the HP bar), rises 14 px with ease-out over the first 35 % of 0.7 s, holds, fades from 60 %; consecutive numbers step 0 / +5 / −5 px sideways; max 32 on screen; text colour `#f2efe6` with the usual 1 px shadow; drawn after lighting | Readable at night, does not cover the HP bar (10 and 24 px starts overlapped it), stacked combo numbers do not sit on top of each other |
| Critical number | bold, gold `#f2b544`, "!" suffix, rises 20 px, lives 0.9 s | Only one 12 px font size exists, so colour, weight, height and time carry the emphasis instead of size |
| Settings | `--shake 0-1`, `--flash 0-1` (default 1) multiply shake distance and flash alpha; 0 turns them off. A settings screen and file come in phase 15 | Spec 6.7 / 13.3 accessibility |

## Monsters (phase 11a)

Code: `client/src/game/presentation/monster_view.cpp`, `monster_pose.cpp`. Art: `content/monsters/slime_green.bmp` from the test-content generator (48×24, two 24×24 frames, feet at 12,21). Everything here only reads the monster's position, state and walked distance.

| Element | Look | Why |
|---|---|---|
| Green slime | dome body `#60ba58` over a darker lower band `#3e8c46`, dark outline `#1c4228`, pale top shine `#c8f4aa` with one brightest pixel, two dark eyes with a white glint; flat bottom | Reads as a soft, harmless first enemy at 32 px tiles; the shine sells "jelly" without transparency (BMP has none) |
| Windup frame (11b) | frame 1: wider (24) and lower (11) with frowning brows | Pose change for the telegraph; drawn now so the art stays in one place |
| Shadow | same soft shadow as the hero, 18×6; shrinks 8 % per pixel of lift (min 50 %) and stays on the ground | Hops read as leaving the ground |
| Breathing (idle) | squash/stretch ±6 % over 1.4 s; each slime is phase-shifted by index × 0.618 | Alive while resting; the golden-ratio step keeps neighbours out of sync |
| Hopping (wander) | one hop per 14 px walked: lift up to 3 px at mid-hop, 16 % taller (8 % thinner) in the air, 10 % flatter on landing | Slimes move in hops; tying the hop to walked distance keeps feet and motion in sync at any speed |
| Hop blend | hop strength eases 0 ↔ 1 at 6/s (about 0.17 s) when walking starts or stops | Stops never snap the body from the air to the ground |
| Pixel crispness | scaled size and position are rounded to whole pixels; the feet (origin) stay on the ground point while scaling | Keeps pixel art from smearing and the slime from sliding while it breathes |
| F4 boxes | hurtbox `#f0d23c` at 100 alpha (same as dummies), feet box `#50c8ff` at 140 alpha (same as the hero), attack hitbox `#f03c3c` at 110 alpha while lunging (11b, same as the sword) | Debug only; drawn at the current fixed-step position (what collision uses) |

### Monster fighting (phase 11b)

Code: `monster_pose.cpp` (`slime_action_pose`), `monster_marks.cpp`, `monster_view.cpp`. Agreed with the developer: the telegraph uses **pose + ground marker only, no body flash**.

| Element | Look | Why |
|---|---|---|
| "!" mark | 3 px wide yellow `#ffd64a` bar (7 px) + dot (3×3) inside one 1 px dark `#10131c` outline, 3 px above the head; pops up 5 px over 0.18 s; shown while noticing and 0.35 s into the chase, fading; drawn after lighting | Classic, readable at 640×360 and at night; one outline rect so the gap row does not darken while fading (two overlapping rects did, 11b screen check) |
| Notice hop | one 4 px hop with a little stretch in the first half of `notice_seconds` | "Surprised" body language that matches the mark |
| Windup pose | frame 1 (wide, low, frowning), squashing a further 8 % wider / 10 % lower over the windup; from 60 % on it shivers ±1 px at 15 Hz | Pose change you can read without a flash; the shiver says "now" just before the lunge |
| Ground telegraph | the soft round light texture stretched along the locked lunge path (from feet + hitbox forward to lunge end + forward, plus the hitbox width at each end) and rotated to it; outer `#e63428`, inner core `#ff805c` at 55 % size and 60 % alpha; grows from 55 % to full size (ease-out) and from alpha 70 to 190; drawn in the Shadows layer (on the ground, under bodies) | Shows exactly where the hit will land so the player can step out (spec 9.6). The path is the real swept area, so trusting it always works |
| Telegraph light | a red light `#ff5440` at the path centre, radius (path + width) × 0.9, intensity 0.35 → 0.9 over the windup | The ground marker is drawn before lighting, so at night it was nearly invisible (11b screen check). The light keeps it visible in the dark and lights the slime ominously; in daylight it changes nothing |
| Lunge | rises up to 7 px (sine) while travelling, 16 % taller and 8 % thinner at the top; shadow stays on the ground and shrinks | The leap reads clearly and its landing spot is the shadow |
| Landing | splats 18 % wider / lower at the start of recovery, back to normal by 30 % | Weight on landing and a visible "opening" |
| Chase / return | same distance-locked hops as wandering, at chase speed | Consistent movement language |

### Monsters getting hit and dying (phase 11d)

Code: `monster_view.cpp`, `monster_pose.cpp`, `enemy_bar_drawing.cpp` (shared with dummies), `field_combat_fx.cpp`; effect `content/fx/slime_burst.json`.

| Element | Look | Why |
|---|---|---|
| Hit flash | same as dummies: the body drawn again additive white, 0.08 s, times `--flash` | One language for "this hit landed" |
| Damage numbers, sparks, shake | same as dummies (`DamageNumbers`, `hit_spark`) | Shared hit feel |
| HP bar | the dummies' bar (26×5, red fill `#d64840`, cream trail), drawn 3 px above the head and following the hop/lunge lift; hidden while full, shown on the first hit | Shows progress toward the kill; moved to `queue_enemy_bar` so both enemies use one drawing |
| Hurt recoil | 16 % thinner and taller at the hit, easing back over the stagger | A jelly wobble that says "that hurt" without a new frame |
| Death | flattens with a quadratic ease (35 % wider, 65 % lower by the end) over 0.5 s while blinking white (70 % additive every 0.1 s, times `--flash`) | Spec 9.5 "blink and vanish"; melting suits a slime |
| Vanish burst | `slime_burst`: 16 soft dots, pale mint `#e4ffd8` → green `#4f9e4a` fading out, 40–110 px/s in all directions with gravity 220, 0.35–0.65 s, 6 px above the feet | The light start colour was chosen because the first green (`#8ee07a`) blended into the meadow grass (11d screen check) |

## Hero getting hit (phase 11c)

Code: `client/src/game/presentation/hurt_vignette.cpp`, `hero_blink.cpp`, `field_combat_fx.cpp`; vignette texture painted by `paint_vignette` (engine render layer). Spec 6.7: a hit on the player uses a red screen-edge vignette and a short invulnerability blink. Everything only reads `HeroHitEvent` and the hit reaction.

| Element | Look | Why |
|---|---|---|
| Red vignette | a 160×90 white texture, transparent inside elliptical distance 0.55, rising with smoothstep to full at 1.25 (corners ~1.41), scaled with linear filtering to 640×360 and tinted `#d61c24`; alpha 0.85 × level², level 1 → 0 over 0.6 s; Foreground layer below the HP bars and "!" marks (after lighting, so it shows at night) | Reads instantly as "you got hurt" without covering the hero in the centre; the squared fade gives a sharp pop that leaves quickly. 160×90 + linear filtering keeps the gradient smooth with no stair steps |
| Blink | while hit-invulnerable (0.6 s) the hero sprite alternates every 0.05 s between alpha 25 % and 100 %; dodge invulnerability does not blink | Classic "can't be hit now" signal; never fully invisible, so the player never loses the hero |
| Shake | the attack's `shake` (slime 0.25) adds trauma like a sword hit | Weight on impact; follows the `--shake` setting |
| Hitstop | the attack's `hitstop` (slime 0.06 s) | Same freeze as hits on enemies, so both sides of the fight feel alike |
| Accessibility | the vignette follows `--flash` (0 hides it); shake follows `--shake` | A full-screen red pulse counts as a flash for photosensitive players |
| No number | no damage number over the hero (agreed with the developer) | Numbers are for enemies; the HUD HP bar (phase 12) will show the hero's HP |

## Hero falling (phase 12b)

Code: `client/src/game/presentation/death_fx.cpp` (timers), `player_sprite.cpp` (lying pose), `client/src/engine/render/desaturate.cpp` + `frame_readback.cpp` + `Renderer::apply_desaturation` (grayscale), `client/src/game/ui/fallen_screen.cpp` + `client/src/app/field_ui.cpp` (fallen screen). All of it only reads `PlayerMotion::dead()`; timers run on frame time.

| Element | Look | Why |
|---|---|---|
| Lying pose | no fall frames exist yet, so the standing frame is rotated about its centre up to 90° over **0.35 s** (`InQuad`: tips slowly, then drops) and lowered **14 px** (rounded to whole pixels). Facing left falls to the left, any other facing to the right. No blink while fallen | The sprite centre is 22 px above the feet; 14 px down puts the lying body on the ground line. Reuses the current art (agreed); a real fall frame can replace it later |
| Grayscale | the world (everything drawn before the HUD, including damage numbers) mixes toward luminance `(77R + 150G + 29B) / 256` by an amount that rises 0 → 1 over **1.5 s** (`OutQuad`). HUD bars and the fallen screen stay in colour | Spec 9.5. Same length as the slow motion. HUD colour kept so the empty red HP bar still reads (agreed) |
| Grayscale method | the 640×360 frame target is read to the CPU (`SDL_RenderReadPixels`), converted to RGBA32, desaturated row by row, uploaded to a streaming texture and drawn back over the target. Only while the hero is down | Chosen with the developer: SDL_Renderer has no shader path. Cost measured on the dev machine (Debug build): about 144 → 66 FPS while fallen. If it fails once, it logs and stays off for the run |
| Fallen screen | after **1.2 s**, a full-width black band (alpha 150, 48 px high, centred at **68 %** of the screen height) and the bold red text "쓰러졌다" (`#d64840`, same red as the HP bar) fade in over **0.8 s** (`OutCubic`) while rising 6 px; then "Enter: 바로 부활" (12c) fades in over 0.4 s (`OutQuad`) | The delay lets the slow motion and grayscale land first. The band sits below the centre because the camera keeps the fallen hero in the middle; the first try (centred band) covered the body |
| Respawn fade (12c) | full-screen black fade, **0.8 s** out (`InQuad`) and **0.8 s** in (`OutQuad`), more than twice the 0.35 s map fade; the new map opens in colour with the area banner and a full HP bar that fills up | A slow "waking up" instead of a quick door change; the gray world fades to black and comes back in colour (agreed) |

## Cat-nip cushion (phase 13b)

Code: `client/src/game/presentation/cushion_view.*`, `player_sprite.*` (rest clips), `client/src/app/rest_hud.*`. Art: `content/props/catnip_cushion.bmp` and row 4 of `content/sprites/test_hero.bmp`, both drawn by `tools/test_content/generate_test_content.ps1`.

- **Cushion sheet** 128×32, 32×32 cells, seat (feet) at (16, 28): 0 withered (grey-mauve cushion, drooping brown leaves), 1 bloomed (lavender cushion, three upright catnip sprigs with lilac buds), 2 a cream "z" with a dark outline, 3 a pink heart. Cushions are Y-sorted 0.5 px before the hero so she always sits on top.
- **Hero poses** (row 4, front view): 0 sit, 1 curled up dozing (eyes closed, head on knees), 2 stretch (arms up, on tiptoe, ^ ^ eyes), 3 sit with ^ ^ eyes. Clips: `sit` = 3 for 0.5 s then 0; `curl` = 1; `stretch` = 2 then 3. She sits during settling and for 1.4 s, then curls up.
- **Bloom:** the sprite switches to bloomed and pops 1 → 1.3 → 1 over 0.45 s (half a sine, scaled around the seat), `fx/catnip_bloom.json` bursts 18 lilac-to-mint additive motes, and a lilac light (radius 72) flashes and fades over 0.8 s.
- **Resting:** a mint light `#78e8a0` (radius 60, intensity 0.85) fades in over 0.6 s and breathes at 0.5 Hz (depth 0.25). Two hearts pop sideways when she sits (0.9 s, rise 12 px); while dozing a "z" rises every 0.8 s (1.6 s, rise 16 px, ±3 px sine sway). Floaters fade over the last 40 % of their life, at most 12 at once, drawn in `WorldFx` above leaves.
- **Text:** "[E] 쉬기" in cream on a translucent band, 64 px above the bottom edge, fades in/out in about 0.17 s and rises 4 px as it appears. "주변이 너무 소란스럽다" in soft orange `#ec9c60` at 62 % screen height for 1.8 s, fading over the last 0.3 s. The discovery banner reuses the area banner.
- Screen check (13b): at night the green light makes the sitting hero look strongly mint; acceptable for now, may be softened when real art arrives.

## Cushion menu and level-up (phase 13c)

Code: `client/src/app/cushion_menu_view.*`, `CushionView::celebrate_level_up`, `fx/level_up.json`.

- Menu: a 168 px panel 24 px from the right edge (the hero stays visible at the centre), title "캣닢 쿠션", four buttons (dimmed ones at 40 % alpha), level and runes below. Panels pop in over 0.2 s (OutBack from 90 %), again when switching menu ↔ level-up.
- Level-up sheet: 300×236 panel; focused row on a faint gold band `#ffe9a8` 16 %; raised values in green `#8edc80`, preview in dim text, short runes in red `#e66054`; a refused confirm shakes the cost line ±4 px for 0.3 s.
- Level-up: `fx/level_up.json` (additive cream-to-gold streaks rising at the hero's feet, a burst every 0.08 s for 0.5 s) and a gold light `#ffdc8c` (radius 80) fading over 1 s (visible only where lighting is on, i.e. at night or indoors). HUD bars follow the new max with an exponential ease (rate 8/s).

## Runes (phase 13c)

Code: `client/src/game/presentation/rune_orbs.*`, `client/src/app/field_ui.cpp` (`draw_runes`).

- Counter at the bottom right (12 px margins): a 5 px gold diamond `#ffd678` with a dark outline, then the number in bold. Gains float "+N" in light gold 10 px up over 1.2 s; losses turn the number and "-N" red `#e66054`.
- Orbs: gold `#ffd678` additive glows 10 px, drawn on the Glow layer (above lighting, so they read at night). Burst 40–90 px/s with drag 5 for 0.3 s, then pulled at 900 px/s² up to 420 px/s toward the hero's body (18 px above the feet); caught within 8 px; at most 64 alive, 2 s life.

## Ground items (phase 14b)

Code: `client/src/game/presentation/loot_view.*`, `client/src/game/ui/loot_toasts.*`, `client/src/app/loot_toast_drawing.*`.

- Icons: `items/item_icons.bmp`, 160×16 (15a: dagger 7, greatsword 8), one 16×16 cell per catalog `icon` index (generator section "아이템 아이콘"). Dark ink outline so they read on grass.
- Drop: the icon flies from the fallen monster's feet to its landing spot in a 0.45 s arc 14 px high (ease-out sideways), then bounces once 4 px for 0.15 s. The 0.6 s total is also when it can first be picked up. The bounce follows the item's simulation age, so items already on the ground when you enter a map are simply lying there.
- Resting: bobs 0–1.5 px (2.4 rad/s, offset per item); a 12×4 shadow that shrinks while it is in the air; every 2.2 s (offset per item) a 9 px additive glint at the icon's top right for 0.3 s (Glow layer).
- Night: each item gives a small 18 px light (intensity 0.55, slow 0.5 Hz pulse) in its rarity colour: common warm white `#fff0c8`, uncommon sky `#78dcff`, rare pink-violet `#ffaaff`.
- Pickup: the icon flies to the hero's body (10 px above the feet) in 0.28 s, accelerating (u²) and shrinking to half size, then `fx/item_pickup.json` (8 warm sparks) bursts there.
- Notice line (bottom left, decided with the developer): band `#00000078`, 16 px icon, text in cream `#f0e8c8` ("슬라임 젤리 ×2") or red-orange `#f07860` ("가방이 가득 찼다 · 롱소드"). Newest line at the bottom (10 px margin), 20 px apart, at most 4 lines. Each slides in 40 px from the left in 0.2 s, stays 2.6 s and fades over the last 0.4 s; picking up the same item within 1 s adds to the count instead of adding a line.

## Bag screen (phase 14c)

Code: `client/src/app/bag_screen_view.*`, colours in `client/src/game/presentation/rarity_color.hpp`.

- A panel on the left of the screen (16 px from the edge, centred vertically), so the heroine and nearby monsters in the middle stay visible while time runs. It slides in 12 px from the left over 0.18 s (OutCubic) while fading in.
- Title "가방" in bold gold, used/total slots on the right in dim text.
- Slots: 26 px cells on a 28 px pitch, 6 × 4. Dark fill `#0000005a`; empty cells have a dim `#5a6070` border, filled cells a border in the item's rarity colour (the same colours as the ground glow). The chosen cell gets a pale gold fill and a gold outline 1 px outside the cell. Counts above 1 sit at the bottom right on a small `#00000096` band so the icon stays readable.
- Details under the grid: name in bold rarity colour with "×N" on the right, then kind and effect ("소비 · HP +50" in green `#8edc80`, materials and weapons in dim text), then the description (up to 3 lines). An empty slot shows "빈 칸".
- Small menu: a 72 px panel right next to the chosen cell with three 18 px buttons (사용 / 버리기 / 취소; the first reads 장착 on equipment, 15a); unavailable choices are drawn at 40 % and skipped.
- Weapon detail line (15a): "무기 · 무게 N" in dim text, or "무기 · 무게 14 / 8 · 무거움" in red `#ec6060` when heavy for the current strength (the heavy check comes from `derive_stats`, not from the view).
- Bottom hint: "R 정렬 · I 닫기" (dim). The first version also listed Enter and was clipped by the 166 px width, so it was shortened.

## Moon blade and star swarm (phase 15c)

- Moon blade: the sword's slash-arc sprite (`fx/slash_arc.bmp`) played once on the cut box, tinted moon blue `#aacdff`, additive on the Glow layer, visible 0.24 s (a little longer than a sword arc so the spell reads as magic). The spell's light (`#dce8ff`, radius 44) sits on the box centre and fades with the arc. Code: `SlashArc::play` / `advance`, owned by `FieldCombatFx`.
- Star swarm stars are ordinary shards to the renderer (glow, trail, end sparkle, light carried per shard), so circling and curving need no new drawing code. Their small size (6 px) and dimmer light (`#9cc8ff`, radius 28, 0.6) keep five of them from washing out the screen.

## Hands HUD (phase 15b)

Code: `client/src/app/hands_hud.*` (owned by `ItemHud`). Spell icons: `ui/spell_icons.bmp`, 128×16, cell = spell `icon` (generator section "마술 아이콘"): 0 star shard, 1 great star shard, 2 star swarm, 3 moon blade, 4 comet beam, each on a navy night-sky disc so they never read as item icons.

- Bottom-left corner (8 px from the left and bottom edges, spec 13.2): a 24 px weapon cell, 4 px gap, a 24 px spell cell, then the spell name (regular text, 110 px wide) with one 4 px dot per open spell slot under it (7 px apart).
- Cells: `#00000078` fill (same darkness as the notice band) with a 1 px `#5a6070c8` border; the 16 px icon is centred. An empty weapon cell means bare hands; no spell shows "마술 없음" in dim text.
- Slot dots: selected slot in strong gold `#ffcc5c`, other filled slots `#9aa3b5`, empty slots outline only. The first try used the theme's focused gold for the selected dot, which was too close to the cream filled dots to tell apart on screen.
- Not enough MP for one cast (beam: one second of MP): spell icon and name drop to 40 % (the same dimming as unavailable bag menu items).
- Q: the spell icon pops to 1.3× and eases back over 0.18 s (OutCubic, spec 6.9 range).
- The item notice line now sits 32 px higher so it stacks above these cells (decided with the developer).

## Drinking (phase 14d)

Code: `client/src/game/presentation/drink_view.*`. There is no drinking frame in the hero sheet yet, so the potion's own 16 px icon shows the action.

- The bottle sits at her hand: 7 px to the side when she faces left or right, 5 px right of centre when facing up or down, 14 px above the feet. Facing up it is drawn behind her, otherwise in front.
- Tilt: up to 70° toward the mouth by the gulp (0.35 s, InOutQuad) while rising 6 px, then back down by 0.6 s (OutQuad). Mirrored when facing left.
- Gulp: `fx/potion_heal.json` (16 soft green motes `#c8ffb4` → `#5ad278`, rising) for HP potions, `fx/potion_mana.json` (same in blue `#c8dcff` → `#5a78f0`) for MP-only potions, from 12 px above the feet; plus a 44 px light in green `#78e68c` or blue `#6e96ff` fading over 0.35 s (visible at night).
- A hit before the gulp removes the bottle at once with no motes.
- Refusals use the red notice line: "지금은 마실 수 없다", "이미 가득하다 · 회복 물약" (repeated identical warnings within 1 s refresh the line instead of stacking).

## Rune trace (phase 14e)

Code: `client/src/game/presentation/rune_trace_view.*`; fallen text in `client/src/app/field_ui.cpp`.

- A gold pool lying on the ground: an additive 26×10 glow `#ffc45a` breathing (alpha 110–170, size +15 %) with a 2.2 rad/s sine (about 2.9 s), and a 9×5 bright core `#ffecaa`.
- Three 4 px gold dots circling it on a flat 9×3.5 px ellipse at 1.3 rad/s, so it reads as lying on the ground.
- `fx/rune_trace.json`: 2 gold pixel motes every 0.25 s, drifting up slowly (0.7–1.2 s).
- A 40 px gold light `#ffce6e` (intensity 0.7) pulsing with the pool, so it can be found at night.
- On grass in daylight the additive gold looks slightly lime; it stays readable, so it was left as is.
- Recovery: the 13c rune orbs burst from the trace and fly into her; the area banner shows "룬을 되찾았다"; the counter rolls up with "+N".
- Fallen screen: the band grew from 48 to 64 px for two hint lines (runes, then "Enter: 바로 부활").

## New monsters (phase 13c)

Art by `tools/test_content/generate_test_content.ps1`: `monsters/bat.bmp` 48×24 (open wings / folded windup, feet 10 px below the body because it flies), `mushroom.bmp` 64×32 (red cap with white dots and a face on the stem / windup with white spikes and a frown; the side spikes are mostly clipped by the cell, the top three show), `frog.bmp` 48×24 (sitting / puffed cheeks and open mouth). Death bursts: `fx/bat_burst.json` (violet), `fx/spore_burst.json` (slowly rising cream spores), frog reuses `slime_burst`.
- Spit: a 12 px green glob `#78c846` with a 6 px additive core `#dcffaa` (Glow layer), interpolated between steps; `fx/spit_splash.json` (10 green droplets) where it ends. The windup warning is the usual red ground path, as long as the spit flies.
- The mushroom warning has no lunge, so the red ground mark becomes a circle as wide as its 52 px spike box.

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
