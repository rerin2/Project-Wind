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
- The first part of a dodge is invulnerable (`Mover::invulnerable`). Since 11c monster hits use it (`PlayerMotion::can_be_hit`); it does not blink.

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

Code: `client/src/game/combat/hit_stop.*`, `sim_time_scale.*` (multiplies hitstop with the 12b slow motion), `client/src/game/states/field_state.cpp`. Tested in `hit_stop_tests.cpp`, `sim_time_scale_tests.cpp`.

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

## Monsters (phase 11a: idle and wander, 11b: fighting)

Code: `client/src/game/monsters/monster.*`, `client/src/game/states/field_combat.*`; data `content/monsters/slime_green.json` (format in `docs/ENGINE.md` "Monster"). Tested in `monster_tests.cpp`, `monster_loader_tests.cpp`.

- A monster is placed by the map's `entities` (`"type": "monster"`). The tile it stands on is its **home**. Monsters do not block tiles. Two green slimes live in the test meadow at tiles (11,19) and (15,21).
- **Idle:** rest for a random time in `wander.pause` (the first rest is `pause` min, so no dice are rolled outside the fixed step).
- **Wander:** pick a random point inside `wander.radius` of home (try up to 8 times; points outside the circle, inside a wall, or less than 1 px away are rerolled; if none fits, rest again). Walk straight to it at `wander.speed`, sliding along walls with the same `move_and_slide` as the hero (corner assist 2 px). On arrival, rest again.
- **Stuck:** a step that covers less than half its expected distance counts as stuck; after **0.5 s** stuck the monster gives up and rests. It never ends up inside a wall (test).
- Points are picked inside a square and rejected outside the circle, so no `sin`/`cos` is needed and the same seed gives the same walk on every machine.
- **Dice:** all monsters share one seeded `CombatRandom` owned by `FieldCombat`, seeded with the field seed XOR a constant, so sword crits never change where monsters walk. Same seed → same walk (test `Monster.SameSeedSameWalk`). The field seed still changes every launch.
- Hitstop (`time_scale` 0) freezes monsters too: a 0 s step changes nothing.
- Since 11c their lunge hurts the hero (see "Hero getting hit"); since 11d they can be hit and killed (see "Monsters getting hit and dying").

| Green slime | Value | Why |
|---|---|---|
| `wander.radius` | 40 px | A little over a tile: it shuffles around its spot instead of roaming the meadow |
| `wander.speed` | 18 px/s | Clearly slower than its chase speed (40) and the hero walk; reads as idle |
| `wander.pause` | 1.0–2.5 s | Rests long enough to look calm, short enough to look alive |
| `body` | 12 × 6 px | Narrower than the hero (14 × 8) so it slides through the same gaps |
| `max_hp`, `attack_power`, `detect_range`, `leash_range` | 30, 40, 120, 260 | Spec 14.2 example values, except `attack_power` (developer set 40 in 11c; spec example 4) |

### Fighting (phase 11b)

Code: `client/src/game/monsters/monster.*`, `monster_moves.*`; data `content/attacks/slime_hop.json`. Tested in `monster_tests.cpp` (`MonsterAi.*`). `FieldCombat` passes the hero's feet to every monster each fixed step.

| State | What happens | Leaves when |
|---|---|---|
| idle / wander | as above | the hero comes within `detect_range` of the monster → notice |
| notice | stands still, "!" over its head | `notice_seconds` passed → chase |
| chase | walks straight at the hero at `move_speed`, sliding along walls (no pathfinding yet, spec 9.6) | the monster is more than `leash_range` from home, **or** the hero is more than `leash_range` from the monster → return; the hero is within `attack_range` → windup |
| windup | stands still; the lunge direction is **locked** toward the hero at this moment; crouch pose and red ground telegraph | attack `windup` passed → attack |
| attack | travels `lunge` px along the locked direction during `active` (sliding along walls); the attack hitbox exists only now and moves with the body | `active` passed → recover |
| recover | stands still (the opening to punish) | `recovery` passed → chase if both leash checks pass, else return |
| return | walks home at `move_speed` | home reached → idle; stuck for **1 s** → idle on the spot; the hero is within `detect_range` **and** within `leash_range − attack_range` of home → notice |

- **Why the direction is locked:** the telegraph shows exactly where the slime will land, so stepping out of the red area always works.
- **Territory rule while returning (agreed 2026-11-17):** a returning monster notices the hero again at once, but only inside its territory (leash minus attack range). Without that margin a hero standing on the leash border would make the slime flip between notice and return.
- **Range rule:** `wander.radius + detect_range ≤ leash_range`, so a monster never notices a hero it would give up on at once.
- Hitstop freezes every state timer (a 0 s step changes nothing).
- **Replay (spec 17.2):** the same seed and the same hero path give the same positions and state sequence, attacks included (`MonsterAi.SameSeedSameHeroSameResult`).
- After a lunge the slime used to end on top of the hero and lunge in an almost random direction next; bodies pushing apart (11c) fixes this.

| Slime hop | Value | Why |
|---|---|---|
| `windup` | 0.6 s | Spec 9.6 "the player must be able to dodge": enough to read the crouch and step out at walking speed (about 70 px) |
| `active` / `lunge` | 0.25 s / 40 px (160 px/s) | Faster than walking, so standing still gets you hit, but short enough that one step sideways is safe |
| `recovery` | 0.7 s | A clear opening to counter-attack |
| hitbox | 18 × 14 px, 6 px ahead, 6 px up | About the slime body; moves with it during the lunge |
| `knockback`, `hitstop`, `shake` | 160, 0.06, 0.25 | Used when the hit lands on the hero (11c) |
| `attack_range` | 40 px (spec example 28) | The lunge reaches 46 px ahead (40 + 6), so it starts when the hero is inside that reach |
| `notice_seconds` | 0.4 s | Long enough to see the "!" hop; short enough not to feel slow |

## Monsters getting hit and dying (phase 11d)

Code: `client/src/game/monsters/monster.*`, `client/src/game/combat/hit_resolution.*` (one template for dummies and monsters), `client/src/game/states/field_combat.*`. Tested in `monster_tests.cpp` (`MonsterHit.*`), `field_combat_tests.cpp`, `field_assets_tests.cpp`, `field_combat_fx_tests.cpp`.

- **Same hit rules as dummies:** the sword's active hitbox and star shards hit monsters through the same `resolve_swing` rules (once per swing, `compute_damage` with the monster's `defense`, crits from the seeded dice). A shard hits the first target only, dummies checked first. Monster target ids follow the dummies' (dummies 1..N, monsters N+1..).
- **HP:** `max_hp` from data (slime 30). The longsword combo 14 → 15 → 21 kills a slime on the third hit (only 1 HP is left for it).
- **Knockback:** real, along the push direction, slowing to 0 over `stagger_seconds` (slime 0.25 s; longsword hit 1 = 90 px/s → about 11 px), with wall sliding. Same `HitReaction` as the hero, without invulnerability (one hit per swing already limits it).
- **Stagger (`hurt`), agreed 2026-11-17:** a hit during the windup **cancels the attack**; a hit during the lunge only pushes and the lunge goes on; in every other state the monster is staggered for `stagger_seconds`. After the stagger it chases if both leash checks pass (so a hit from outside the detect range still makes it come), otherwise it returns home.
- **Death:** at HP 0 the state becomes `dead`: no movement, no attack, cannot be hit, not part of body pushing. After **0.5 s** it is `gone` (not drawn); `FieldCombat::vanished()` reports that step so presentation bursts `death_fx` (and runes/drops can hook in later, phases 13–14).
- **Respawn (spec 9.5, until the catnip cushion in phase 13):** every map entry rebuilds the field from the data files, so a slime killed earlier is back at home with full HP when you re-enter the map (`FieldAssets.ReenteringMapBringsSlimesBack`).
- **Replay (spec 17.2):** the same seed and the same per-step hero input record (walk, attack, dodge) give identical hero and slime positions, HP and state sequences, with hits on both sides (`FieldCombat.SameSeedSameInputsSameFight`).

## Hero getting hit (phase 11c)

Code: `client/src/game/entities/health.*`, `hit_reaction.*`, `player_motion.*`, `client/src/game/states/field_combat.*` (`resolve_monster_attacks`). Tested in `health_tests.cpp`, `hit_reaction_tests.cpp`, `player_motion_tests.cpp`, `field_combat_tests.cpp`.

- **When:** each fixed step after monsters move, a lunging monster's attack hitbox is checked against the hero hurtbox (14 × 30 px above the feet). A hit needs: overlap, the hero not invulnerable, and the first hit of this lunge (`Monster::claim_hit`). If the hero is invulnerable the lunge stays "unused", so it can still land if the invulnerability ends while the lunge is active.
- **Damage:** `compute_damage(monster attack_power × attack power − hero defense)`; hero defense is 0 until equipment (phase 15). Green slime: `attack_power` × 1.0 − 0. The data now has `attack_power` **40** (changed by the developer in the 11c commit; it was 4), so a lunge deals 40 and three lunges make the hero fall (40 + 40 + the last 20). Tests read the value from the data file, so tuning it does not break them.
- **HP:** max **100** (spec 8.2, vigor 10). Since 12b HP goes down to **0** and the hero falls (see "Hero falling"). The 11c temporary floor of 1 is gone; `HealthTuning::floor` stays only for tests.
- **Stagger (`hit`):** **0.3 s**. The state table blocks attacking, dodging and casting; the legs stop; a running attack and a cast are cancelled (MP already paid is not refunded); walking or dodge momentum stops. Buttons pressed during the stagger stay in the 0.15 s input buffer.
- **Knockback:** along the lunge direction, starting at the attack's `knockback` (slime 160 px/s) and slowing evenly to 0 at the end of the stagger, so the distance is knockback × stagger ÷ 2 (**24 px**). It moves the feet box with `move_and_slide`, so a wall stops it on one axis and the hero slides along the wall on the other.
- **Invulnerability:** **0.6 s** after a hit (spec 9.4). The first **0.16 s** of a dodge are invulnerable too (`dodge_invulnerable_seconds`, pulled into 11c from phase 12, agreed).
- **Hitstop:** the hit's `hitstop` (slime 0.06 s) freezes the field like a sword hit; the longer freeze of the step wins (`FieldCombat::strongest_hitstop`).
- No damage number over the hero (agreed); F3 shows `hp 96/100`. The HUD HP bar comes in phase 12.

| Hero hit | Value | Why |
|---|---|---|
| stagger | 0.3 s | Long enough to feel the hit and see the push; short enough that the 0.6 s invulnerability leaves time to act before the next hit |
| invulnerability | 0.6 s | Spec 9.4 default; also how long the red vignette fades |
| hurtbox | 14 × 30 px, from 30 px above the feet | Body of the 32 × 48 sprite without the ears and hair edge, same width as the feet box |

## Hero falling and respawning (phases 12b, 12c, spec 9.5)

Code: `client/src/game/entities/player_motion.*` (`dead`, `step_dead`, `vitals`, `restore`), `health.*`, `client/src/game/combat/slow_motion.*`, `sim_time_scale.*`, `client/src/game/states/field_combat.*` (`hero_fell`), `client/src/game/world/respawn_trigger.*`, `map_travel.*`, `client/src/app/client_app.cpp` (respawn and carrying vitals). Tested in `respawn_trigger_tests.cpp`, `map_travel_tests.cpp`, `screen_fade_tests.cpp`, `health_tests.cpp`, `player_motion_tests.cpp`, `slow_motion_tests.cpp`, `sim_time_scale_tests.cpp`, `field_combat_tests.cpp`.

- **When:** a hit takes HP to **0** (damage above the remaining HP only removes what is left). The hero's state becomes `dead` before anything else, even during the stagger.
- **While dead:** buttons do nothing (presses expire from the 0.15 s buffer), the legs stop, stamina and MP do not refill, and the hero cannot be hit (`can_be_hit` is false, `take_hit` returns 0). Only the knockback of the fatal hit keeps sliding to its end (24 px for a slime), so the body lands where the hit pushed it.
- **Monsters:** they see no hero (`std::nullopt`). A chasing, winding-up or recovering monster gives up and walks home, then rests and wanders as usual. The fallen body is left out of body separation, so monsters can walk over it.
- **Slow motion:** the step after the fatal hit, the simulation time scale drops to **0.3** and returns to **1** over **1.5 s** of real step time with `InQuad` (stays slow for most of the time, then catches up). It is a time scale like hitstop (spec 3.2): `sim_dt = step × hitstop_scale × slow_scale` (`SimTimeScale`). If the fatal hit also has hitstop, the freeze comes first and the slow motion continues after it. Knockback distance and projectile paths are the same as at normal speed, only slower (tested).
- **A fallen body never leaves the map:** exits are not checked while the hero is dead, so knockback that pushes the body onto an exit tile does not start a map change.
- **When to respawn (12c, agreed 2026-11-22):** `RespawnTrigger` starts counting once the fallen screen is fully shown (2.0 s after falling). **Enter** respawns at once; otherwise it respawns by itself after **2.5 s**. If the map cannot be opened the hero stays down and the trigger fires again 2.5 s later.
- **Where:** the last cat-nip cushion rested at (13b, may be on another map; the hero wakes up sitting on it), or the current map's `default` spawn if she has not rested yet. The map is read again, so HP, MP and stamina start full and defeated monsters are back (spec 9.5). Rune loss: see "Stats, runes and levels" (13a).
- **Carrying HP between maps (12c, agreed):** a normal exit no longer refills the hero. Before the new field replaces the old one, `HeroVitals` (HP, MP, stamina, exhausted) is read from the old hero and given to the new one (`PlayerMotion::restore`, every value clamped to its range). Only a respawn starts full. This closes the "walk between maps to heal" hole noted in 12a; phase 16 can save the same values.

| Falling and respawning | Value | Why |
|---|---|---|
| slow start | 0.3× | Slow enough to watch the fall and the last knockback; not a full stop, so the world still feels alive (agreed) |
| slow length | 1.5 s real time, `InQuad` back to 1× | Matches the grayscale fade, so the world reaches full speed just as it turns fully gray (agreed) |
| auto respawn | 2.5 s after the fallen screen is fully shown (4.5 s after falling) | Long enough to read "쓰러졌다" and feel the loss; short enough not to stall. Never stuck if no key is pressed (agreed) |
| respawn key | Enter (`Confirm`) | Skips the wait; not a combat key, so a panicked attack press cannot skip the fallen screen |

## Stats, runes and levels (phase 13a, spec 8.2, 8.3)

Code: `shared/include/wind/progression/*` (`Stats`, `StatCurve`, `derive_stats`, `level_cost`, `RuneWallet`), `shared/src/content/progression_loader.cpp`, `client/src/game/progression/hero_progress.*`, `hero_tuning.*`, `FieldCombat::collect_runes`/`set_hero_stats`. Data: `content/progression/stat_curves.json`, `levels.json` (formats in `docs/ENGINE.md`). Tested in `stats_tests.cpp`, `stat_curve_tests.cpp`, `derived_stats_tests.cpp`, `level_cost_tests.cpp`, `rune_wallet_tests.cpp`, `progression_loader_tests.cpp`, `hero_progress_tests.cpp`, `hero_tuning_tests.cpp`, `field_combat_tests.cpp`.

- **Five stats**, each 1–60, all start at **10** (agreed 2026-11-22). Level = sum − 49 (start 1, max 251), never stored.
- **Derived values** come only from `derive_stats` (one function, spec 22.2): max HP/MP/stamina, carry weight, strength and intelligence bonus, spell slots. The start values match the fixed numbers used up to phase 12 (HP 100, MP 50, stamina 100, bonuses 0), so nothing changes for a new hero. The field builds the hero's `Health`/`Mana`/`Stamina` maximums from them (`hero_tuning`); the sword uses the strength bonus and the star shard the intelligence bonus.
- **Curves outside their points stay flat** (13a): a stat below the first point (10) gets the first value. Stats can only go up from 10, so this only matters for test data. Whole-number values (max HP, MP, stamina, spell slots) are floored.
- **Level-up cost** from L to L+1 = `floor(100 + 40·(L−1) + 6·(L−1)²)` (1→2: 100, 10→11: 946, 30→31: 6,306, 250→251: 382,066). Raising several levels at once adds each step.
- **Raising stats** (`HeroProgress::try_raise`, screen in 13c) is all-or-nothing: the target may not lower any stat or pass 60; if the runes are short nothing changes.
- **Runes:** wallet 0–999,999,999 (extra is dropped). A monster gives its `rune_reward` once, on the fixed step its HP reaches 0 (sword or shard). The green slime gives **12** (spec 14.2 example), so level 2 takes 9 slimes.
- **Falling** loses `floor(runes ÷ 2)` on the step of the fatal hit; stats and level stay. Lost runes cannot be recovered yet: recovering them from a rune trace at the death spot is decided (spec 8.3.1, decision 0010) and planned for phase 14e. If a kill and the fall land on the same step, the runes are added first, then halved.
- **Owner:** `HeroProgress` lives in `ClientApp`, outside the field, so stats and runes survive map travel and respawn (agreed 2026-11-22). The field borrows it. Phase 16 saves only the five stats and runes.

## Cat-nip cushions (phase 13b, spec 8.7)

Code: `client/src/game/world/rest_rules.*`, `cushion.hpp`, `travel_targets.*`, `client/src/game/entities/hero_rest.*`, `PlayerMotion::begin_rest`/`sit_on`/`stand_up`, `client/src/game/states/field_rest.*`, `FieldCombat::monster_engaged`, `HeroProgress::rest_at`, `client/src/app/field_session.*`. Tested in `rest_rules_tests.cpp`, `hero_rest_tests.cpp`, `field_rest_tests.cpp`, `player_motion_tests.cpp`, `travel_targets_tests.cpp`, `hero_progress_tests.cpp`, `field_combat_tests.cpp`, `entity_loader_tests.cpp`, `map_loader_tests.cpp`.

- **Placement:** map entity `save_point` (see `docs/ENGINE.md`). The meadow cushion "초원 입구" is at (5, 10), up and left of the start, more than 7 tiles from every slime; the forest cushion "숲 그늘" is at (6, 11) near the entrance.
- **Rest:** stand within **24 px** of the cushion seat (tile feet point) and press **E**. "[E] 쉬기" shows while close enough and able to rest.
- **Too noisy:** if any monster is engaged (Notice, Chase, Windup, Attack, Recover or Hurt; not Return) or the hero was hit less than **3 s** ago, nothing happens except the notice "주변이 너무 소란스럽다". Far from every cushion, E does nothing.
- **Order (simulation):** 1) the hero must be idle/moving/running (state table); her move force stops, buffered presses are dropped, **HP, MP and stamina refill and exhaustion clears** at once, she faces down and slides to the seat in 0.25 s, then sits (`resting`: no actions, cannot be hit, exits ignored). 2) 0.6 s after sitting the field asks for a rest trip: 0.5 s fade, the same map is read again (monsters back, spec 9.5), the hero starts sitting on the cushion. 3) The cushion becomes the respawn point (`last_cushion`), the cushion is remembered as found. 4) Autosave: logged only until phase 16. 5) The cushion menu comes in 13c.
- **Leaving:** after a rest the cushion menu opens (13c); "떠나기" or Esc makes her stretch (0.6 s) and stand. When she wakes on a cushion after falling there is no menu: E or any direction key makes her stand (13b rule).
- **Falling** respawns on the last cushion (another map if needed), sitting. Before any rest it is the current map's `default` spawn as in 12c.
- **First discovery:** a found cushion is drawn bloomed; the first rest at a withered cushion makes it bloom and shows "캣닢 쿠션을 찾았다 — (이름)" (the name defaults to the area name).

| Cushion | Value | Why |
|---|---|---|
| reach | 24 px | Spec 8.7; about the hero's body width, so she must really stand at it |
| calm after a hit | 3 s | Spec 8.7; stops "rest to heal" in the middle of a fight |
| settle | 0.25 s | Quick enough to feel responsive, long enough to see her step onto it |
| wait before the rest fade | 0.6 s | Shows her sitting down and the hearts before the screen darkens |
| rest fade | 0.5 s out + 0.5 s in | "Briefly dark" (agreed), shorter than the 0.8 s respawn fade |
| stretch | 0.6 s | Long enough for the stretch frames; short so leaving feels snappy |

## Cushion menu and level-up (phase 13c, spec 8.3, 8.7, 13.1)

Code: `client/src/game/ui/cushion_menu.*`, `level_up_sheet.*`, `client/src/app/cushion_menu_panel.*`, `cushion_menu_view.*`, `FieldState::refresh_hero_stats`, `PlayerMotion::raise_maximums`. Tested in `cushion_menu_tests.cpp`, `level_up_sheet_tests.cpp`, `health_tests.cpp`, `hud_bar_layout_tests.cpp`.

- Opens by itself after the rest fade. Items: 레벨업, 마술 칸 (dimmed until 15), 저장 (dimmed until 16), 떠나기. Up/Down skip dimmed items; Enter, E or J confirm; Esc leaves. **Game time stops** while it is open.
- **Level-up screen:** five stat rows and a 확정 row. Left/Right (or Enter on a stat row) lower/raise the target between the current value and 60; lowering below the current value is impossible. Each changed row previews its derived value ("최대 HP 100 → 118", weight with one decimal, magic attack). Cost = the sum of each level step (13a); shown red when short. 확정 is all-or-nothing (`HeroProgress::try_raise`); a refusal shakes the cost line. Esc returns to the menu and drops the unconfirmed change.
- **After a level-up:** new max HP/MP/stamina apply at once and refill (she is resting), the sword and spell bonuses update, a "레벨 N" banner shows, the bars grow over about 0.4 s.

## Runes on screen (phase 13c, spec 8.3, 13.2)

Code: `client/src/game/ui/rune_counter.*`, `client/src/game/presentation/rune_orbs.*`, `FieldCombat::rune_drops`, `HeroProgress::last_lost`. Tested in `rune_counter_tests.cpp`, `rune_orbs_tests.cpp`.

- Bottom right: rune count. A gain shows "+N" (gains in a row add up) and rolls up in 0.6 s; a loss rolls down red in 0.8 s with "-N". The first value shows at once.
- A falling monster drops 3–10 gold motes (about one per 4 runes) that scatter for 0.3 s, then fly into the hero; the wallet already has the runes.
- The fallen screen hint reads "잃은 룬 N   Enter: 바로 부활".

## New monsters (phase 13c)

Data: `content/monsters/bat.json`, `mushroom.json`, `frog.json` and their attacks. Tested in `enemy_shots_tests.cpp`, `monster_tests.cpp`, `monster_senses_tests.cpp`, `monster_loader_tests.cpp`.

| Monster | HP | Attack | Def | Speed | Runes | Behaviour |
|---|---|---|---|---|---|---|
| 숲 박쥐 `bat` (forest ×3) | 18 | 18 | 0 | 70 | 8 | Notices at 140 px after 0.25 s, short 0.35 s warning, 64 px dive. Weak but quick |
| 가시 버섯 `mushroom` (forest ×2) | 70 | 30 × 1.2 | 4 | 22 | 20 | Barely moves (wander 16 px). Within 34 px: 0.9 s warning, then spikes in a 52×52 box around it (hits from any side), long 1.0 s recovery to punish |
| 침 뱉는 개구리 `frog` (forest ×2, meadow ×2) | 26 | 20 | 0 | 45 | 14 | Keeps distance: closer than 70 px it hops back; within 130 px a 0.7 s warning (puffed cheeks, red line 200 px long) then one spit (170 px/s, 200 px). Rolling through the spit avoids it |

## Bodies pushing apart (phase 11c, spec 7.3)

Code: `client/src/game/physics/body_separation.*`, `FieldCombat::separate`. Tested in `body_separation_tests.cpp`, `field_combat_tests.cpp`.

- Bodies are circles at the feet with radius = feet box width ÷ 2 (hero 7, slime 6). After hits are resolved each fixed step, hero ↔ monster pairs, then monster ↔ monster pairs (fixed order, so replays match) that overlap are moved apart.
- **Force limit:** at most **120 px/s** of correction per pair (2 px per step), so bodies slide apart instead of popping, and a fast dodge or lunge can still pass through.
- **Shares:** hero 1 : monster 3 (the hero moves 25 %, a slime 75 %); two monsters split evenly. Each body moves with `move_and_slide`, so nobody is pushed into a wall.
- Hitstop (0 s step) pushes nothing.
- This fixes the 11b issue where a slime ended on top of the hero and lunged in a random direction next.

## Character states

Code: `client/src/game/entities/character_state.*`; spec 8.5. Tested in `character_state_tests.cpp`. The table says which changes are ever allowed; conditions (cancel window, cast timing) are checked by the code that owns that state. Staying in the same state is always allowed. Exhaustion is a stamina flag, not a state, because an exhausted hero can still walk.

| From | Can go to |
|---|---|
| idle, moving, running | idle, moving, running, attacking, casting, drinking, dodging, hit, resting, dead, transitioning, in_dialogue |
| attacking | idle, moving, running, attacking (next combo hit), dodging (cancel window), hit, dead |
| casting | idle, moving, running, dodging (before the cast or while holding a beam), hit, dead |
| drinking (14d) | idle, moving, running, hit (cancels the drink), dead |
| dodging | idle, moving, running, hit, dead |
| hit | idle, moving, running, hit, dead |
| resting, in_dialogue | idle (game input is blocked meanwhile) |
| dead | transitioning, idle (respawn) |
| transitioning | idle |

For now the hero uses idle, moving, running, attacking, casting, drinking (14d), dodging, hit (11c), dead (12b) and resting (13b); F3 shows the state and the combo hit (`attacking 2`).

## Time

- The game clock runs at 60 real seconds per game hour (a game day is 24 real minutes), starting at 09:00 for a new game. It advances in the fixed simulation step and continues across map travel. Saving the time comes with saves (phase 16).
- Time and weather are presentation only for now: nothing in the rules depends on them yet.

## Items and the bag (phase 14a, spec 10.1, 10.2)

- Items: 회복 물약 (HP +50), 별빛 물약 (MP +30), four monster materials (슬라임 젤리, 박쥐 날개, 버섯 가시, 개구리 침샘) and 롱소드 (a weapon item; equipping comes in phase 15).
- The bag has 24 slots. Potions stack to 20, materials to 99, weapons do not stack.
- Picking up fills matching stacks first, then the first empty slot. If not everything fits, nothing is picked up and the item stays on the ground (phase 14b shows a notice).
- Removing items (using, quests, shops) takes from the last matching slot first and is all-or-nothing.
- Sorting merges stacks and orders items as listed in the catalog.
- Decided with the developer (2026-11-23): items left on the ground stay on their map (also after resting, falling or leaving); the bag screen opens with **I** and the game keeps running while it is open.

## Drops and pickup (phase 14b, spec 9.7)

- Each monster has a drop table (`drops` in its data). When it falls, every line rolls on its own: it drops with `chance`, and then a count from `count` (each count equally likely). A separate drop dice (seeded per field) is used, so drops never change combat or monster movement.
- Drop rates (decided with the developer, "normal"): slime 50 % 슬라임 젤리 ×1–2, 15 % 회복 물약; bat 50 % 박쥐 날개, 10 % 별빛 물약; mushroom 50 % 버섯 가시 ×1–2, 15 % 회복 물약, 5 % 롱소드; frog 50 % 개구리 침샘, 15 % 별빛 물약. With 40 meadow slimes you get about six potions.
- Each drop lands 10–20 px from the fallen monster in a random direction (on its feet if that spot is a wall or water).
- Pickup is automatic: once an item has been on the ground for 0.6 s (its bounce), it goes into the bag when the hero's feet are within 36 px. Each ground item has its own number in that map, so it can be picked up only once.
- If the whole stack does not fit, it stays on the ground and the notice line says "가방이 가득 찼다" once. Walking more than 48 px away and coming back warns again.
- No pickup while fallen or resting.
- Ground items stay on their map when you leave, rest or respawn. Each map keeps at most 64; when a 65th drops, the oldest one disappears.
- A map whose monsters drop an item missing from the catalog does not open (the error is logged).
- Test option `--fill-bag <item_id>` fills every bag slot with that item.

## Bag screen (phase 14c, spec 10.2, 13.1)

- **I** opens and closes the bag. It cannot open while changing maps, fallen, resting or in the cushion menu, and it closes by itself if one of those starts.
- The game keeps running while it is open (decided with the developer), but the heroine stands still: the game loop gives the field no input, so monsters can still reach her.
- Arrows/WASD pick a slot in the 6 × 4 grid (stopping at the edges). **Enter**/E/J on an item opens a small menu: 사용 (use; dimmed until potions can be drunk in 14d), 버리기 (put down), 취소. **R** sorts. **Esc** closes the small menu, then the bag.
- The screen never changes the bag itself: put down and sort are requests the field carries out on the next fixed step, all-or-nothing (an emptied slot does nothing).
- **Put down** places the whole stack 14 px in front of the heroine (at her feet if that spot is a wall or water). It is not picked up again until she has been more than 48 px away.
- **사용** (14d) is offered for consumables. Choosing it closes the bag so you can see her drink (decided with the developer).

## Drinking potions (phase 14d)

- Using a potion takes one from the bag at the start and plays a 0.6 s drink (state `drinking`). At 0.35 s (the gulp) it restores its `heal_hp` / `heal_mp`, capped at the maximum.
- While drinking she walks at 0.4× speed (like casting), cannot run, and cannot attack, cast or dodge. Buttons pressed before the drink are forgotten.
- A hit before the gulp cancels the drink: the potion is used up and nothing is restored (decided with the developer, for soulslike tension). A hit after the gulp keeps the healing.
- She can only start a drink while standing, walking or running. Otherwise the notice line says "지금은 마실 수 없다" and the bag is unchanged. A potion that would restore nothing (HP and, for MP potions, MP already full) is not drunk: "이미 가득하다 · 회복 물약".
- The request goes through the fixed step like put down (`FieldItems::use`): check first, then take one and start, so a refused use never changes the bag.

## Not yet defined

More spells, Q and spell slots (15), quick item slots (15), rune traces (14e), equipment (15), and saves (16) are defined later (spec section 19).
