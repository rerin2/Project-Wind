# Gameplay Rules

Status: phase 6. Spec references: master spec sections 7–9.

This file holds the rules the simulation enforces and the numbers behind them. Change a number here and in code together.

## Movement

Code: `client/src/game/entities/mover.*`, `movement_tuning.hpp`, `client/src/game/physics/tile_collision.*`.

- The simulation runs at a fixed **60 Hz**. Positions are float world pixels; the player's position is the **feet** point.
- Input is an 8-way unit vector. Diagonals are normalized, so diagonal speed equals straight speed (tested).
- Each step, velocity moves toward `input × max_speed` by at most `acceleration × dt` (or `deceleration × dt` when there is no input).
- **Collision body:** a feet box 14 × 8 px whose bottom-centre is the feet point. The sprite does not collide; only the feet box does.
- **Tiles:** a tile blocks if its walkability is false (from the tileset, see ENGINE.md). Outside the map is blocked.
- **Wall sliding:** movement is applied in sub-steps of ≤ 1 px, x first, then y. A blocked axis stops and its velocity becomes 0; the other axis continues.
- **Corner assist:** when moving along a single axis and blocked, the body is nudged sideways by up to the step length if an opening is found within **6 px**. Diagonal movement does not use corner assist.
- **Facing:** follows the held direction (not the velocity), picked from 4 directions by the dominant axis; an exact diagonal keeps the current facing when it is one of the two components.

| Tuning | Value | Why |
|---|---|---|
| `max_speed` | 110 px/s (≈ 3.4 tiles/s) | Brisk but readable on a 640×360 view (≈ 20 tiles wide) |
| `acceleration` | 1100 px/s² | Reaches full speed in ≈ 0.1 s: responsive without feeling weightless |
| `deceleration` | 1400 px/s² | Stops in ≈ 0.08 s, a little faster than starting, so stopping feels precise |
| `corner_assist` | 6 px | Covers small misalignment when entering 1-tile gaps |
| feet box | 14 × 8 px | Narrower than a 32 px tile so 1-tile paths are easy to enter |

## Dodge

- Pressing dodge records it in the input buffer for **0.15 s**. A dodge starts on the first step where it is allowed.
- Direction: the held direction, or the facing direction when nothing is held.
- During a dodge, velocity is fixed at `dodge_direction × dodge_speed`; walls still stop it.
- When it ends, velocity becomes `dodge_direction × max_speed` (no sudden stop) and a cooldown starts.
- The first part of a dodge is invulnerable (`Mover::invulnerable`), used by combat in phase 12 (player survival).

| Tuning | Value | Why |
|---|---|---|
| `dodge_speed` | 280 px/s | ≈ 2.5× walking speed |
| `dodge_seconds` | 0.20 s | Covers ≈ 56 px (a bit under 2 tiles) |
| `dodge_cooldown_seconds` | 0.30 s | Prevents chaining dodges into fast travel |
| `dodge_invulnerable_seconds` | 0.16 s | Most of the roll; the tail end is vulnerable |
| input buffer | 0.15 s | Spec 5.8 default |

## Stamina and running

Code: `client/src/game/entities/stamina.*`, `player_motion.*`; spec 7.2.1. Tested in `stamina_tests.cpp` and `player_motion_tests.cpp`.

- **Run:** hold Shift while moving. Top speed becomes `max_speed × 1.5` = **165 px/s**; letting go eases back to 110 px/s at the normal acceleration. Running drains stamina only while actually moving; holding Shift while standing costs nothing.
- **Dodge cost:** a dodge needs and spends **25** stamina. With less than 25 the buffered press waits (0.15 s) and is then dropped.
- **Regeneration:** after the last spend, stamina waits **0.6 s**, then refills at **30/s** (also while walking). A long step splits into "remaining wait" and "refill".
- **Exhaustion:** reaching 0 makes the hero exhausted; running and dodging stay off until stamina is back to **25**. This stops running from flickering on and off near 0. The HUD bar turns red and blinks while exhausted.
- Stamina never leaves 0…max (boundary tests). Negative spends are ignored. Stamina is not saved and resets to full on map entry for now (phase 16 moves it into the player state).

| Tuning | Value | Why |
|---|---|---|
| `max` | 100 | Spec 7.2.1 start value (endurance 10 in phase 13) |
| `run_multiplier` | 1.5 (165 px/s) | Clearly faster than walking but still under the 280 px/s dodge |
| `run_cost_per_second` | 25 | 4 s of continuous running from full |
| `dodge_cost` | 25 | Four dodges from full; dodge-spamming empties the bar quickly |
| `regen_delay_seconds` | 0.6 | Short pause so stopping to breathe matters |
| `regen_per_second` | 30 | Full again in ≈ 3.9 s (including the wait) |
| `exhausted_until` | 25 | Enough for one dodge as soon as exhaustion ends |

## MP (phase 9d)

Code: `client/src/game/entities/mana.*`, owned by `PlayerMotion`; spec 8.2. Tested in `mana_tests.cpp` and `player_motion_tests.cpp`.

- MP pays for spells (phase 9e casts the first one). It starts full and refills at **2/s** every fixed step, with no wait after spending and no exhaustion.
- Spending is all-or-nothing: `try_spend(cost)` with less than `cost` changes nothing and returns false, so a spell you cannot afford does not go off. Negative costs are ignored.
- MP never leaves 0…max (boundary tests). Like stamina it resets to full on map entry for now; phase 16 saves `mp_current`.

| Tuning | Value | Why |
|---|---|---|
| `max` | 50 | Spec 8.2 curve at intelligence/mind 10; fixed until phase 13 |
| `regen_per_second` | 2 | Spec 8.2; slow on purpose: 25 s from empty to full, so spells are a resource and resting refills them |

## Star shard numbers (phase 9d data, cast since 9e)

`content/spells/star_shard.json`. Spec 8.6 gives MP 6, cast 0.25 s, power 1.0, intelligence 10; the flight values below were chosen in 9d (agreed with the developer).

| Value | Number | Why |
|---|---|---|
| `speed` | 360 px/s | Clearly faster than running (165) and dodging (280), so it reads as a shot; 6 px per fixed step, equal to its size, so it cannot skip a 1-tile wall |
| `range` | 240 px | 7.5 tiles: reaches across most of the 640×360 view but not off screen |
| `size` | 6 × 6 px | A small spark; smaller than a dummy hurtbox (20×34) so aim matters a little |
| cost / regen | 6 MP of 50 | 8 shots from full; 3 s of regen per shot |
| damage | 12 | magic attack 12 × (1 + 0 intelligence bonus) × power 1.0 against defense 0 |
| moving while casting | slow walk, 0.4 × 110 = 44 px/s | The heavy "great star shard" is the one that roots you (spec 8.6); the basic spell keeps you mobile. 0.4 is slow enough that casting is a commitment but you can still step out of a slow attack |

## Casting (phase 9e)

Code: `client/src/game/combat/spell_caster.*`, `projectile.*`, `client/src/game/entities/player_motion.*`, `client/src/game/states/field_combat.*`; spec 8.6, 9.2. Tested in `spell_caster_tests.cpp`, `projectile_tests.cpp`, `player_motion_tests.cpp`, `field_combat_tests.cpp` (the last one with the real `star_shard.json`).

- **Keys:** L or C. The press goes into the input buffer (0.15 s) like attack and dodge.
- **Start:** allowed from idle, moving or running (state table). Not while attacking, dodging or already casting; the press then waits in the buffer and expires. One spell is fixed for now (star shard); Q and spell slots come in phase 15.
- **MP is paid at the start, all or nothing.** With enough MP it is spent and the cast starts. With too little, nothing is spent, no cast starts, the press is used up (it does not fire later by itself), and a counter goes up so the HUD MP bar blinks.
- **Cast time:** `cast_seconds` (0.25 s = 15 fixed steps). The spell goes off on the step the time is reached. The direction is fixed when the cast starts (the pressed direction, else the facing) and the hero keeps facing it while casting.
- **While casting:** walk at 0.4 × speed, no running, no attack, no dodge (spec 8.5: only before a cast or while holding a beam; beams come later). Only projectile spells can be cast yet; melee and beam spells load but do nothing until phase 15.
- **The shard** starts 10 px in front of the feet and flies at chest height (14 px above its ground point). Damage = magic attack 12 × (1 + intelligence bonus 0) × power, then spec 9.3 (`magic_attack_power`, `compute_damage`).

| Spawn value | Number | Why |
|---|---|---|
| forward | 10 px | Just past the hero's body box, so it does not start inside the hero |
| lift | 14 px | Chest height: hits a dummy hurtbox (34 px tall) whether you face it from the side or from above/below |

### Flying shard rules

- Each fixed step the shard moves `speed × dt`, split into steps of at most half its smallest side (3 px for the star shard), so a fast shot cannot jump over a 1-tile wall or a thin target.
- After every small step it checks, in order: **target** (its body-height rect against standing dummy hurtboxes; the first one hit takes damage and the shard ends), then **wall** (its ground point's tile), then **range** (`range` px flown in total).
- A tile blocked only by a standing entity (a dummy) is **not** a wall for shards, otherwise the shard would vanish just before the dummy. Out of the map counts as a wall.
- Known limitation: shards use the walking map, so water and other "not walkable" ground also stop them. A separate "blocks shots" tile flag can come with the first map that needs it.

## Damage

Code: `shared/include/wind/combat/damage.hpp`; spec 8.2, 9.3. Tested in `shared/tests/damage_tests.cpp`.

- **Physical attack power** = weapon `base_power` × (1 + `strength_scaling` × strength bonus). Strength 10 gives bonus 0, so the longsword hits with 14 at the start.
- **One hit**, in this order:
  1. Target invulnerable (dodge, after being hit) → **no hit**, 0 damage. This check comes first, so the minimum does not apply.
  2. Negative, infinite or NaN inputs count as 0 (bad data can never heal or explode).
  3. `raw = attack_power × power`.
  4. `max(minimum, raw − defense)`.
  5. Critical → × `crit_multiplier` (after defense).
  6. Round **down**, then clamp to `minimum…maximum`.
- Critical hits are decided by the caller (see "Critical hits" below).

| Rule | Value | Why |
|---|---|---|
| `minimum` | 1 | A hit always does something, even through heavy defense |
| `maximum` | 99,999 | Far above any planned number; keeps the int conversion safe |
| `crit_multiplier` | 1.5 | Noticeable without one-shotting |

## Critical hits (phase 10a)

Code: `client/src/game/combat/combat_random.*`, `hit_resolution.*`, `client/src/game/states/field_combat.*`. Tested in `combat_random_tests.cpp`, `field_combat_tests.cpp`.

- Every hit that lands rolls once: chance = **5 %** + the weapon's `crit_bonus` (sword), or 5 % (spells). Chosen with the developer; the dagger's +10 % (spec 8.4) makes 15 %.
- The roll uses `CombatRandom`, a seeded splitmix64 generator written out in code, so the same seed gives the same crits on every compiler (spec 9.3). It is separate from the particle RNG, so effects can never change combat. The field seeds it randomly on entry; tests pass a fixed seed (or chance 0 to check exact damage).
- A critical hit deals × 1.5 after defense (longsword 1st hit 14 → 21). F3 marks it with `!` (`last hit 21!`); the big coloured number comes in 10b.

## Knockback (phase 10a)

Code: `client/src/game/combat/knock_sway.*`, `training_dummy.*`. Tested in `knock_sway_tests.cpp`, `field_combat_tests.cpp`.

- The hit pushes along the swing direction (the hero's facing) or the shard's flight direction, with the attack's or spell's `knockback` (px/s) as starting speed.
- **Training dummies** stand on a blocked tile, so they are never moved off it (chosen with the developer). Their body sways instead: a damped spring pulls the offset back to 0. The offset is simulation state: the hurtbox moves with it, the feet/tile do not. Real knockback that moves a character through the map (with wall sliding) comes with monsters in phase 11.

| Spring | Value | Why |
|---|---|---|
| `stiffness` | 300 /s² | Returns in about a third of a second, snappy like a padded post |
| `damping` | 14 /s | One small overshoot, then still |
| `max_offset` | 8 px | A quarter tile: clearly visible, never leaves the tile; the finisher (160) reaches it |

Longsword hit 1 (90) peaks at about 4 px; hit 3 (160) at about 7–8 px; the star shard (40) at about 2 px.

## Hitstop (phase 10a)

Code: `client/src/game/combat/hit_stop.*`, `client/src/game/states/field_state.cpp`. Tested in `hit_stop_tests.cpp`.

- When a hit lands, the field simulation (hero, dummies, shards) freezes for the attack's `hitstop` seconds. Spec 3.2: the only way it reaches the simulation is the time scale, `sim_dt = step × time_scale` (0 while frozen, 1 otherwise). Nothing is moved or skipped; frozen steps simply advance by 0.
- The freeze counts down in real step time and starts on the step after the hit. A second hit during a freeze keeps the **longer** of the two (no stacking); values are capped at 0.5 s.
- Button presses during the freeze stay in the input buffer (its timer does not run), so a combo press during hitstop still chains.
- Time of day and presentation (particles, later shake and numbers) keep running.

| Data | Hitstop | Frozen steps (60 Hz) |
|---|---|---|
| longsword 1 / 2 | 0.05 s | 3 |
| longsword 3 | 0.09 s | 6 |
| star shard | 0.03 s | 2 |

## Longsword combo

Data: `content/weapons/longsword.json`, `content/attacks/longsword_{1,2,3}.json`. The first two hits use the spec's "normal" timing (0.08 / 0.10 / 0.18 s); the finisher is slower and stronger.

| Hit | windup / active / recovery (s) | cancel window (s) | power | knockback | hitbox (fwd, lift, len × wid) |
|---|---|---|---|---|---|
| 1 | 0.08 / 0.10 / 0.18 | 0.18–0.36 | 1.0 | 90 | 18, −8, 28 × 24 |
| 2 | 0.08 / 0.10 / 0.18 | 0.18–0.36 | 1.1 | 90 | 18, −8, 28 × 26 |
| 3 | 0.14 / 0.12 / 0.28 | 0.36–0.54 | 1.5 | 160 | 20, −8, 32 × 30 |

Each swing costs 14 stamina (spec 8.4). The cancel window opens when the hit's active part ends, so the next hit can follow (or a dodge can cut the recovery) without skipping the hit itself. The weapon is loaded with the field (`weapons/longsword.json`, the start weapon); if it fails to load, the error is logged and the hero is bare-handed (attacks do nothing, everything else works).

## Attacking

Code: `client/src/game/combat/attack_runner.*`, `client/src/game/entities/player_motion.*`; spec 7.2.1, 9.2. Tested in `attack_runner_tests.cpp` and `player_motion_tests.cpp`.

- **Keys:** J or Z. The press goes into the same 0.15 s input buffer as the dodge.
- **Starting:** a swing starts when the state table allows it (not while dodging), the runner is free, and stamina is above 0 and not exhausted. It spends the weapon's `stamina_per_attack` (14); with less left the swing still happens and stamina drops to 0 (exhaustion starts). At 0/exhausted the press waits in the buffer and is then dropped.
- **Direction:** the held direction at the start, otherwise the current facing. The facing then stays fixed for the swing.
- **Legs:** while attacking the hero's input is ignored for movement (she slows to a stop at the normal deceleration) and cannot run.
- **Combo:** a buffered press inside the current hit's cancel window starts the next hit. After the last hit, or when a swing ends without a press, the next attack starts again from hit 1.
- **Dodge cancel:** during a swing a dodge only comes out inside the cancel window; it ends the swing at once. If both buttons are buffered in the same step, the dodge wins.
- **Hits:** the hitbox exists only during `active`. Each target can be hit **once per swing**; the next combo hit is a new swing and can hit it again.

## Training dummy (phase 9c)

Code: `client/src/game/combat/training_dummy.*`, `hit_resolution.*`, `client/src/game/states/field_combat.*`; data `content/props/training_dummy.json`. Tested in `training_dummy_tests.cpp`, `hit_resolution_tests.cpp`, `field_combat_tests.cpp` (with the real longsword data).

- Placed by the map's `entities` (two in the test meadow, tiles (10,14) and (13,14)). A dummy **blocks its tile**: the hero cannot walk through it.
- **Each fixed step, after the hero moves:** if the sword is in its `active` part, every standing dummy whose hurtbox overlaps the hitbox and has not been hit by this swing takes damage. Damage uses `compute_damage` (see Damage): attack power = longsword 14 × (1 + 0.8 × strength bonus 0) = 14, times the hit's power, minus the dummy's defense 0.
- With the longsword the combo deals **14 → 15 → 21** (50 per full combo); a 100 HP dummy falls after two combos.
- **HP 0:** the dummy is knocked down for **1.5 s** (it cannot be hit meanwhile), then stands up with full HP. It never disappears, so it can be practised on forever.
- One swing can hit several dummies if the hitbox reaches them.
- Strength bonus is fixed at 0 until stats arrive in phase 13. No crits, knockback, hitstop or damage numbers yet (phase 10).

| Dummy | Value | Why |
|---|---|---|
| `max_hp` | 100 | Two full longsword combos: enough to see the bar go down step by step |
| `defense` | 0 | Shows the raw weapon numbers |
| `recover_seconds` | 1.5 | Long enough to see it knocked down, short enough to keep training |
| hurtbox | 20 × 34 px above the feet | Covers the straw body and head of the 48 px sprite |

## Character states

Code: `client/src/game/entities/character_state.*`; spec 8.5. Tested in `character_state_tests.cpp`. The table says which changes are ever allowed; conditions (cancel window, cast timing) are checked by the code that owns that state. Staying in the same state is always allowed. Exhaustion is a stamina flag, not a state, because an exhausted hero can still walk.

| From | Can go to |
|---|---|
| idle, moving, running | idle, moving, running, attacking, casting, dodging, hit, resting, dead, transitioning, in_dialogue |
| attacking | idle, moving, running, attacking (next combo hit), dodging (cancel window), hit, dead |
| casting | idle, moving, running, dodging (before the cast or while holding a beam), hit, dead |
| dodging | idle, moving, running, hit, dead |
| hit | idle, moving, running, hit, dead |
| resting, in_dialogue | idle (game input is blocked meanwhile) |
| dead | transitioning, idle (respawn) |
| transitioning | idle |

For now the hero uses idle, moving, running, attacking, casting and dodging; F3 shows the state and the combo hit (`attacking 2`).

## Time

- The game clock runs at 60 real seconds per game hour (a game day is 24 real minutes), starting at 09:00 for a new game. It advances in the fixed simulation step and continues across map travel. Saving the time comes with saves (phase 16).
- Time and weather are presentation only for now: nothing in the rules depends on them yet.

## Not yet defined

The rest of combat (hit feel, monsters, survival in 10–12; more spells, Q and spell slots in 15), growth and items (13–15), and saves (16) are defined later (spec section 19).
