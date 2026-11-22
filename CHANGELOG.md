# Changelog

## Unreleased

### Phase 13b — cat-nip cushions
- Map entity `save_point` (no data file, optional `display_name`, walkable, id may not equal a spawn id; ids unique
  across `content/maps`, tested). Meadow cushion at (5, 10), forest cushion at (6, 11).
- Rest rules (`rest_rules`): within 24 px, not while a monster is engaged (`FieldCombat::monster_engaged`) or within
  3 s of a hit. `FieldRest` handles E: sit (`PlayerMotion::begin_rest` refills HP/MP/stamina, `resting` state, cannot
  be hit) or stand up; "too noisy" counter; rest trip request 0.6 s after sitting. `HeroRest` timeline
  (settle 0.25 s, sit, stretch 0.6 s). `PlayerMotion::seconds_since_hit`.
- New `Action::Interact` on E (also `--press e@N`).
- `TravelKind` (Exit, Respawn, Rest) replaces the respawn flag in `MapTravel`. Rest: 0.5 s fade, same map reloaded
  with the cushion id as spawn, hero starts sitting. `HeroProgress::rest_at`/`last_cushion`/`discovered`; respawn goes
  to the last cushion (`travel_targets`).
- Visuals: `CushionView` (withered/bloomed cushion, bloom pop + `fx/catnip_bloom.json` + lilac flash, mint rest light,
  hearts and "z"), hero clips `sit`/`curl`/`stretch` (new sprite row 4, clip `"row"` field), `RestHud`
  ("[E] 쉬기", "주변이 너무 소란스럽다"), discovery banner.
- Cleanups (no behavior change): travel moved from `ClientApp` into `FieldSession` (317 → 272 lines), exit watching
  moved from `FieldState` into `ExitWatcher`; generator draws the front head through shared functions (rows 0–3
  pixel-identical).
- 575 CTest tests.

### Phase 13a — growth rules (stats, runes, level cost)
- New SDL-free rules in `shared/…/wind/progression/`: `Stats` (five stats 1–60, start 10, level = sum − 49),
  `StatCurve` (points joined by lines, flat outside), `derive_stats` (the one place derived values are computed),
  `level_up_cost`/`cost_between` (`floor(a + b·(L−1) + c·(L−1)²)`), `RuneWallet` (cap 999,999,999,
  all-or-nothing spending, `lose_half`).
- New data `content/progression/stat_curves.json`, `levels.json` and loader `wind/content/progression_loader`;
  the client will not start without them.
- Monster data: required `rune_reward` (green slime 12).
- `game::HeroProgress` (owned by `ClientApp`, survives travel and respawn): stats, runes, `derived`, `preview`,
  `cost_to`, all-or-nothing `try_raise`, `earn`, `fall`. `hero_tuning` turns derived values into the hero's
  `Health`/`Mana`/`Stamina` maximums.
- `FieldCombat`: `set_hero_stats` (strength/intelligence bonus replace the fixed 0), `runes_earned` (once per
  monster, on the step it falls). The field adds runes and halves them when the hero falls.
- `field_state.cpp` kept under 300 lines by collapsing four one-line loops (no behavior change).
- 545 CTest tests.

### Phase 12c — respawn
- `RespawnTrigger`: respawn on Enter or 2.5 s after the fallen screen is fully shown; retries if still down.
- Respawn goes through `MapTravel` (`TravelOptions{respawn, fade_seconds}`): slow 0.8 s fade out and in, current
  map at its `default` spawn, fresh field (full HP/MP/stamina, monsters back). `EnterMap` gets a `respawn` flag.
- `ScreenFade::fade_out(style, seconds)`: per-trip length reused by the next `fade_in`.
- `HeroVitals` (HP, MP, stamina, exhausted) carried into the new field on normal exits
  (`PlayerMotion::vitals`/`restore`, `Health`/`Mana`/`Stamina::restore`), closing the heal-by-travel hole.
- A dead hero no longer triggers exits. Fallen-screen hint: "Enter: 바로 부활".
- Build fix: `scripts/dev_environment.ps1` sets `VSLANG=1033` so Ninja's `/showIncludes` prefix matches and
  header dependencies are recorded (before, every object had 0 deps and header edits left stale objects).
- 503 CTest tests.

### Phase 12b — hero falls
- HP floor is now 0 (`HealthTuning::floor` default 0, `Health::empty`). `PlayerMotion::dead()`; state `dead`
  wins over `hit`. While dead: no actions, no movement, no stamina/MP regen, cannot be hit; the fatal hit's
  knockback still finishes.
- `FieldCombat`: monsters see no hero once it is dead (they give up and go home); the fallen body is not
  pushed; `HeroHitEvent::fatal` and `FieldCombat::hero_fell()`.
- `SlowMotion` (0.3× → 1× over 1.5 s, `InQuad`) and `SimTimeScale` (hitstop × slow motion) replace the
  field's bare `HitStop` member.
- Presentation: `DeathFx` (grayscale 1.5 s, lying 0.35 s), lying pose in `PlayerSprite::queue` (rotate 90°,
  drop 14 px), world-only grayscale via CPU readback (`Renderer::apply_desaturation`, `frame_readback`,
  `desaturate_rgba`), "쓰러졌다" screen (`FallenScreen`, band at 68 % height).
- Temporary (12b only): Enter on the fallen screen reloads the current map at `default`.
- `renderer.cpp`: screenshot saving moved to `frame_readback`; screen textures made by one helper.
- 495 CTest tests.

### Phase 12a — HUD HP bar
- HUD HP bar (delayed bar, red `#d64840`, cream trail, never fades) on the top row; the bars now go HP · MP ·
  stamina from the top (spec 13.2). The stamina bar moved from y = 12 to y = 30.
- `game/ui/hud_bar_layout` (`hud_bar_y`, `hud_bar_inner_width`): one place for row positions and the
  max × 0.8 px length rule (clamped 8–200, whole pixels, safe for NaN). Stamina length now also follows its max.
- `FieldUi` draws all three bars with one `draw_bar` helper; `HudInfo` gains `hp`, `hp_max`, `stamina_max`;
  `FieldState::health()` exposes the hero HP to the HUD.
- 471 CTest tests.

### Phase 11d — hitting and defeating slimes
- `resolve_swing` is now one template shared by dummies and monsters; the sword and star shards hit monsters
  (shards: first target, dummies first). Monster target ids follow the dummies'.
- `Monster`: HP, `take_damage`, `push`, states `hurt` and `dead`, `gone` after 0.5 s. Real knockback with wall
  sliding over `stagger_seconds`. A hit cancels a windup, does not stop a lunge, and otherwise staggers; afterwards
  the monster chases (aggro on hit) or returns. Dead monsters skip AI, hits and body pushing.
- Data: monster `stagger_seconds` (0–2) and optional `death_fx`; `content/fx/slime_burst.json` (pale mint → green).
  `FieldEffects::monster_fx` and `load_monster_effects`. `FieldCombat::vanished()` events burst the effect.
- Presentation: monster hit flash, HP bar (enemy bar drawing moved to shared `queue_enemy_bar`), hurt recoil,
  death melt with white blink, vanish burst. F3 last hit shows a monster's HP and "(dead)".
- Respawn on map re-entry (fresh field) and the per-step input replay test (spec 17.2).
- The slime-hits-hero test now derives its expected damage from the data (the developer set `attack_power` 40).
- 467 CTest tests. Phase 11 complete.

### Phase 11c — the hero gets hit
- `Health` (max 100; temporary floor 1 until death in phase 12) and `HitReaction` (0.3 s stagger, 0.6 s
  invulnerability, knockback slowing to 0 over the stagger: 160 px/s → 24 px). `PlayerMotion::take_hit` cancels
  attack, cast and dodge momentum; state `hit` blocks every action; knockback moves the feet with wall sliding
  (`Mover::shove`, `Mover::halt`).
- `FieldCombat` resolves monster lunges against the hero hurtbox (once per lunge via `Monster::claim_hit`, skipped
  while invulnerable) and records `HeroHitEvent`s; `strongest_hitstop` feeds hitstop for both sides.
- Dodge invulnerability (first 0.16 s) now protects from hits (pulled in from phase 12).
- Bodies push apart (spec 7.3): `separate_bodies` on feet circles, hero 1 : monster 3, 120 px/s per pair.
  Fixes the 11b slime-on-top-of-hero issue.
- Presentation: red screen-edge vignette (`HurtVignette`, new `Renderer::create_vignette_texture`), invulnerability
  blink (`hero_blink_alpha`, `PlayerSprite::queue` alpha), shake and hitstop from the attack data. No damage number
  over the hero (agreed). F3 shows `hp 96/100`.
- 452 CTest tests.

### Phase 11b — slimes notice, chase, telegraph and lunge
- `Monster` state machine: idle / wander → notice ("!", `notice_seconds`) → chase → windup (direction locked) →
  attack (lunge) → recover → chase or return. Gives up when the monster is over `leash_range` from home or the hero
  is over `leash_range` from it; while returning it notices again only inside its territory (leash − attack range).
  `FieldCombat` passes the hero's feet each step. Walking helpers split into `monster_moves` (`step_toward`,
  `step_along`).
- Data: attack field `lunge` (0–256 px over `active`); monster fields `notice_seconds` and `attacks` (1–6 files,
  loaded through `AttackSource`, now in `attack_loader.hpp`); new rule `wander.radius + detect_range ≤ leash_range`.
  `content/attacks/slime_hop.json` (0.6 s windup, 40 px lunge in 0.25 s, 0.7 s recovery); slime `attack_range` 40.
- Presentation: "!" mark, notice hop, crouch frame with growing squash and late shiver, red ground telegraph along
  the locked lunge path plus a red light so it stays visible at night, lunge arc, landing splat, F4 attack hitbox.
  No body flash (agreed). F3 shows the busiest monster state (`mobs 2 (chase)`).
- Replay test: same seed + same hero path → same positions and states, attacks included.
- 430 CTest tests. The lunge does not hurt the hero yet (11c).

### Phase 11a — monster data, idle and wander
- Phase 11 split into 11a–11d (spec 19). The hero's hit reaction (stagger, i-frame blink, red vignette, real
  knockback) moves from phase 12 into 11c.
- Monster data `content/monsters/slime_green.json` and validated `monster_loader` (ranges; attack ≤ detect ≤ leash;
  wander radius ≤ leash; pause `[min, max]`). `HurtboxShape` and `SpriteStrip` moved to their own headers and read by
  shared `body_readers` (dummy and monster).
- Map entity type `monster` (does not block its tile). Every entity must now stand on a walkable tile.
- `Monster` (client `game/monsters`): home, idle ↔ wander inside a radius, wall sliding, gives up after 0.5 s stuck,
  frozen by hitstop. `FieldCombat` owns monsters and a separate seeded monster dice stream. A broken monster file
  drops only that monster.
- Presentation: `MonsterView` + `slime_pose` (breathing, distance-locked hops with squash/stretch, shrinking shadow,
  F4 hurtbox and feet box). Generated slime art (normal + windup frame). Two slimes in the test meadow.
- F3 combat line shows `mobs N (M walk)`.
- 415 CTest tests.

### Phase 10b — hit feel (presentation)
- `FieldCombatFx` gathers the combat views (dummies, slash, spell) and turns each hit into: white additive flash
  (0.08 s), screen shake (`ScreenShake`, trauma² × 6 px on a drawing copy of the camera), hit sparks
  (`fx/hit_spark.json`, twice on crits) and damage numbers (`DamageNumbers`; crits bold gold with "!").
  `field_state.cpp` 299 → 290 lines.
- Accessibility options `--shake 0-1`, `--flash 0-1` (`FeelSettings`); 0 turns them off.
- 392 CTest tests. Phase 10 complete.

### Phase 10a — hitstop, knockback, crits (simulation)
- `HitStop`: a landed hit freezes the field simulation for the attack's `hitstop`; it reaches the simulation only as
  `time_scale` (`sim_dt = step × scale`). Longer freeze wins, capped at 0.5 s.
- `KnockSway`: training dummies sway up to 8 px toward the hit on a damped spring; the hurtbox follows, the tile stays.
- Crits: 5 % + weapon `crit_bonus`, × 1.5, rolled by the seeded `CombatRandom` (splitmix64). F3 marks crits with `!`.
- `HitEvent` now carries critical, direction, knockback, hitstop and shake for 10b. Spells gain optional `knockback`,
  `hitstop`, `shake` (star shard 40 / 0.03 / 0.05).
- 381 CTest tests.

### Phase 9e — casting the star shard
- L / C cast (`Action::Cast`, keys L and C, scriptable `--press l@N`). `SpellCaster` times the cast; `PlayerMotion`
  pays MP up front (all or nothing, refused casts counted for the HUD), locks the direction, walks at 0.4× while
  casting and blocks attack / dodge / run.
- `Projectile`: flies in sub-steps of half its size, checks target → wall → range, ignores tiles blocked only by
  entities. `FieldCombat` spawns shards on release and resolves hits (first target only). `magic_attack_power`.
- Presentation: `SpellView` (hand glow, twinkling halo + core, `fx/star_trail.json`, `fx/star_burst.json`, a light per
  shard via `FieldLighting::set_moving_lights`, purple F4 box), MP bar red blink when a cast is refused, F3 `mp` and
  `shards`, key help shows L.
- Behaviour-preserving split of `field_state.cpp` (305 → 293 lines): `FieldCamera`, `log_field_entry`,
  `queue_shadow` (also used by `CombatView`). Phase 9 complete.
- 362 CTest tests.

### Phase 9d — MP and spell data
- `Mana` (max 50, +2/s, all-or-nothing `try_spend`), owned by `PlayerMotion`; HUD MP bar under the stamina bar
  (0.8 px per point, sky blue `#5fa3d0`, delayed trail).
- `shared/combat`: `Spell` (projectile / melee / beam, MP per cast or per second, optional fx, sfx, light) and
  `ProjectileData`; `AttackData` gains optional `projectile`.
- Validated `spell_loader` (exactly one MP field, exactly one motion, per-second MP only on beams). Hitbox and
  projectile parsing moved to shared `combat_readers` (used by attacks and spells).
- Star shard data `content/spells/star_shard.json` (6 MP, 0.25 s cast, power 1.0, 360 px/s, 240 px, 6×6).
- Phase 9 split into 9a–9e; casting and flying shards move to 9e.
- 333 CTest tests.

### Phase 9c — training dummy and hits
- Map `entities` (optional): `training_dummy` with id, tile and data file; validated (known type, unique id, inside
  the map, not on a spawn/exit/other entity, on a walkable tile). A dummy blocks its tile.
- Dummy data `content/props/training_dummy.json` (HP 100, defense 0, hurtbox, 1.5 s recovery, sprite) and loader.
- `TrainingDummy` (HP, knockdown, stand up with full HP), `resolve_swing` (hitbox ∩ hurtbox, once per swing,
  `compute_damage`), `FieldCombat` (field dummies, one check per fixed step). Longsword combo hits 14 → 15 → 21.
- Presentation: generated dummy art (standing / knocked down), shadow, delayed HP bar above the head (hidden while
  full), white additive slash arc that sweeps over the hit window, F4 hitbox (red) and hurtboxes (yellow), F3 combat line.
- Two dummies in the test meadow. `map_checks` split out of `map_loader.cpp` (325 → 268 lines); `engine::tile_feet`
  shared by spawns and dummies; `DelayedBar` gains `start_hidden`.
- `AGENT.md` records repeated tool mistakes (CLAUDE.md rule 9).
- 313 CTest tests.

### Phase 9b — attack runner, combo and state table
- `AttackRunner`: windup → active → recovery, cancel window, combo order (1 → 2 → 3, then back to 1), one hit per
  target per swing. `place_hitbox` turns the right-facing hitbox into a rect for any facing.
- `CharacterState` and `can_transition` (spec 8.5 table). `PlayerMotion` uses it: attack via the input buffer,
  14 stamina per swing (allowed while above 0, can empty it), legs stop while attacking, dodge cancels only in the window.
- J / Z attack key; the field loads the longsword (bare hands if it fails). F3 shows `attacking N`.
- 286 CTest tests.

### Phase 9a — attack and weapon data, damage
- `shared/combat`: `AttackData` (windup / active / recovery, cancel window, hitbox, power, knockback, hitstop, shake,
  optional mp_cost, fx, sfx, next_combo), `Weapon`, pure `compute_damage` and `physical_attack_power` (spec 9.3, 8.2).
- Validated loaders `attack_loader` and `weapon_loader`; a weapon loads its combo attacks and checks their order.
- Longsword data: `content/weapons/longsword.json` and three attacks (`content/attacks/longsword_1..3.json`).
- 261 CTest tests.

### Phase 8 — text, UI and stamina
- SDL3_ttf `release-3.2.2` (static, vendored FreeType 2.13.2 only) and the Galmuri11 Korean pixel font
  (v2.40.4, OFL 1.1, regular + bold) at 12 px with mono hinting. Licences and the FreeType credit line in DEPENDENCIES.md.
- `TextRenderer` (SDL_ttf, cached text), SDL-free `utf8` helpers and `layout_text`: wrap at spaces, break long words
  by glyph, align, clip with `...`; clipped UI text is counted on F3 and logged once.
- UI: theme file `content/ui/theme.json` with loader, generated 9-slice frames (`content/ui/ui_frames.bmp`),
  `nine_slice`, `FocusList`, `UiPainter`; F7 UI test panel (buttons with focus, blocks game input, Esc closes it).
- Area name banner on map entry; maps now have Korean display names (시험 초원 / 시험 동굴 / 시험 숲).
  The "cannot enter" travel notice is now Korean.
- Stamina (spec 7.2.1): `Stamina`, `PlayerMotion`; Shift run at 165 px/s (25/s), dodge costs 25, 0.6 s regen delay,
  30/s regen, exhaustion until 25. HUD stamina bar with delayed trail, auto-dim when full, red blink when exhausted.
- `PlayerSprite` split out of `field_state.cpp` (364 → 316 lines); `ui_resources` and `transition_view` split out of
  `client_app.cpp`. F3 panel now starts hidden (HUD is in that corner) and shows stamina and UI text stats;
  its help is two lines so it fits 640 px. Key names table completed.
- Test option `--hold key@from-to`; `--press` accepts f7, shift, enter, esc.
- 240 CTest tests.

### Phase 7 — living world and region travel
- Tileset `animation` (frames + seconds per frame) and `sway` (pixels + period). Water shimmers (4 → 14 → 15 → 14),
  flowers lean (9 ↔ 16), tree canopies and tall grass sway 1 px in a wave of wind. Neighbouring cells are staggered.
- Exit `transition`: `fade` (default) or `iris` (a pixel-exact circle closing on the hero, opening on her in the new map).
  Meadow ↔ cave uses iris.
- New test forest (36×24): forest floor, tall grass, mushrooms, dark trees, a shimmering stream, bush border,
  falling leaves, foggy weather table. Reached from the right end of the meadow path (fade).
- `MapTravel` (SDL-free) runs cover → enter → uncover. A bad exit target keeps the current map, logs the reason and
  shows a red notice for 4 s. Tests cover the bad target and follow every shipped exit.
- Tileset parsing moved to `tileset_loader` (map_loader.cpp 360 → 307 lines); `JsonReader::schema_version`.
  `load_field_assets` moved into `wind_game_core` so it can be tested. Test key helpers moved to `scripted_input`.
- 202 CTest tests.

### Fix: F6 seemed to do nothing
- F6 used the table's 5 s fade out + 5 s fade in, so from clear nothing visible happened for 5 s.
  F6 now fades 0.4 s each way; natural weather changes keep the table's slower fade.
- F3 shows a change in progress (`clear -> rain`); the log records each F6 change (or says the map has no weather).
- Test option `--press <key>@<frame>` taps a key at a frame. Verified clear → rain → snow on screen.
- 182 CTest tests.

### Phase 6 — time and weather
- Game clock (60 real s = 1 game hour, starts 09:00) owned by the game loop, so time continues across maps. F5 skips 3 hours.
- Day cycle files (`content/time/`): colour + night keys blended over the day, wrapping past midnight.
- Weather tables (`content/weather/`): weighted states with durations, effects, tint and fog; 5 s fades between states.
  F6 next weather. The meadow has clear, rain (streaks + splashes), snow and fog.
- `FieldAtmosphere`: outdoor ambient = time colour × weather tint; night effects (fireflies) scale with night.
  Indoor maps with a fixed `ambient_light` ignore time and weather.
- Lanterns (tile 13) with `night_only` lights along the meadow path.
- New particle shape `streak`; emitter `rate_scale`; `Glow` draw layer after lighting for additive particles.
- Shared `JsonReader::range`, `color`, `content_path` replace copies in three loaders. Effect `rate` cap 500 → 2000.
- `--time`, `--weather` options. Field loading moved to `field_assets`.
- 181 CTest tests.

### Way into the test cave (early part of phase 7)
- Map `exits` (area in tiles, `to_map`, `to_spawn`) with validation, including "at least one walkable tile".
- Walking onto an exit fades to black, loads the target map at the named spawn, and fades back in
  (`ScreenFade`, 0.35 s each way). If the target cannot load, the hero stays and the error is logged.
- Exits arm only after stepping off exit tiles, so arriving next to one never bounces back.
- Meadow: cave mouth (new tile 12) at the left end of the dirt path; cave: arch exit back on its left wall.
- `FieldState::create` takes a spawn id; each field frees its generated textures when replaced.
- Test option `--auto-move <dir>`; verified meadow → cave → meadow with screenshots.
- 153 CTest tests.

### Phase 5 — lighting
- Light map pass on the SDL renderer: ambient colour, lights added (`ADD`), then multiplied over the frame (`MOD`).
  Foreground and debug layers draw after it. Decision: ADR 0008.
- Engine `light` module: `Light`, flicker and pulse animation, `queue_lights` (off-screen cull, 64 per frame cap).
- Map `ambience` gains `ambient_light`, `player_light` and `lights` (with optional `flicker`, `pulse`, `fx`);
  parsed by the new `ambience_loader`. Colour parsing moved to shared `hex_color`.
- `FieldLighting`: map lights, a light that follows the hero, ember sparks at torches.
- Content: torch tile (11), `fx/torch_embers.json`, test cave map `maps/test_cave.json`.
- F3 shows light counts; `--light-stress N`: 42 lights (37 drawn) at 60.0 FPS.
- Refactors for the 300-line rule: `surface_painter` (procedural textures), `field_effects` (effect loading),
  shared `log_content_errors`.
- 147 CTest tests.

### Phase 4 — particle system
- Particle effects are data: `content/fx/*.json` with validation (file + field path errors). Fields: shape, blend,
  lifetime/speed/size ranges, direction, spread, gravity, drag, spawn radius, colour gradient, fade in/out,
  wobble, burst, rate.
- Engine: `ParticleEffect` + loader, `ParticleRandom`, `make_particle`/`emit_burst`, continuous `ParticleEmitter`,
  colour/fade/wobble drawing with `soft` and `pixel` shapes and per-particle blend mode.
- Dust now uses `fx/dust_walk.json` and `fx/dust_dodge.json`. Broken effect files are logged and skipped.
- Maps can list `ambience.ambient_fx`; the test meadow has drifting petals and fireflies (`WorldFx` layer, above canopies).
- `JsonReader::number` (decimals) and `JsonReader::raw`; `Renderer::create_solid_texture`.
- `--fx-stress N`: 2039 particles measured at 60.0 FPS (VSync), 1654 draw calls, 0 dropped.
- 131 CTest tests.

### Hero redesign
- The hero test sprite is now a silver-haired girl with cat ears and long straight hair: navy dress, violet eyes,
  1 px dark outline, hair tips sway while walking. Right-facing frames mirror the left-facing ones.
- Only `content/sprites/test_hero.bmp` changed; tiles and the test map regenerate byte-identical.
- F3 panel help line mentions F12.

### Phase 3 — rendering foundation
- The frame is drawn into a 640×360 frame target, then scaled to the window. Verified pixel-identical
  to direct drawing outside the F3 panel.
- `SpriteDraw::tint` (colour multiply + opacity, replaces `alpha`) and `SpriteDraw::blend` (`Normal`, `Add`, `Multiply`).
- Tween library: `Ease` curves (`linear`, quad, cubic, `out_back`, `out_elastic`) and `Tween`.
- Screenshots: F12 or `--screenshot-at N` (repeatable), `--screenshot-dir`; default `%APPDATA%\ProjectWIND\screenshots`;
  640×360 BMP; white flash and label afterwards.
- `SdlPlatform::user_data_dir()`.
- `docs/VISUALS.md`: 32-colour palette draft, blend modes, screenshot feedback.
- 105 CTest tests.

### Spec 2.1 — finer phase plan
- Spec section 19 now has 21 phases (0–20) with a summary table; phases 0–2 stay complete.
- Spec section 22 replaced by one common prompt plus per-phase cautions.
- Phase references updated in GAMEPLAY, VISUALS, ENGINE, ADR 0007.

### Phase 2 — cleanup and free movement
- Removed the game server, ENet, `run_server.ps1`, and server tests (single-player, ADR 0007).
- Split files to one class per file pair: `ErrorList`/`LoadResult`/`JsonReader`/`json_file`,
  `AnimationClip`/`SpriteSheet`/`Animator`, `Action`/`InputBindings`/`MovementIntent`. Tests split to match.
- Free 8-way movement (`Mover`): acceleration, deceleration, normalized diagonals, float positions.
- Feet-box tile collision (`move_and_slide`): 1 px sub-steps, wall sliding, 6 px corner assist, map edge blocks.
- Dodge roll (Space/K/X): 280 px/s for 0.2 s, 0.3 s cooldown, early invulnerability flag; 0.15 s input buffer.
- Simulation/presentation split: input → fixed 60 Hz steps → per-frame effects; drawing interpolates between steps.
- Effects: feet shadow, walking dust and dodge dust burst (`ParticlePool`, 512 cap), dodge afterimages,
  camera look-ahead, walk animation speed follows movement speed.
- Debug overlay: feet position, state, speed, particle count/drops; F4 also shows the feet box.
- `Shadows` draw layer; `Renderer::create_soft_ellipse_texture`; empty source rect draws the whole texture.
- New docs: `docs/GAMEPLAY.md` (movement rules), `docs/VISUALS.md` (effect values).
- 94 CTest tests.

### Direction change — single-player action RPG (spec 2.0)
- `docs/MASTER_SPEC.md` rewritten: single player, no server/network/accounts/database,
  free 8-direction movement, real-time action combat, visuals (lighting, particles,
  day/night, weather, game feel, transitions) as a first-class goal, local JSON saves.
- New phase plan (spec section 19): phases 2–8. Phases 0–1 stay complete.
- `CLAUDE.md` updated to match. ADR 0007 added; ADR 0004 superseded; ADR 0001 ENet parts superseded.
- No code changed yet. Server and ENet removal is the first task of phase 2.
- Added the class-per-file module rules (spec 2.4, CLAUDE.md).

### Phase 1 — minimal engine
- Client frame loop in spec order with a fixed 60 Hz update step (0.25 s / 5-step catch-up cap).
- 640×360 logical resolution, integer scaling, nearest-neighbour textures, pixel-snapped camera.
- Engine modules: `Camera2D` (follow, bounds clamp), `InputState` (held/pressed/released, UI capture),
  data-driven sprite animation, sorted `RenderQueue` (layer → feet y → insertion), visible-tile culling.
- `Renderer` wraps SDL3 (sprites, rects, debug text, BMP textures with magenta colour key).
- `ResourceManager`: content-root-confined paths, cache, placeholder texture on failure, stats.
- Shared map/tileset model and JSON validation with file + field-path errors (`wind/content`, `wind/world`).
- Local tile movement (`GridMover`), key bindings (arrows/WASD, F3 debug, F4 collision, Esc quit).
- Debug overlay: FPS, frame time, map/tile/facing, draw calls, culled, textures, cache hits, load failures.
- Test content generator (`tools/test_content`) and a 40×30 test map with original placeholder art.
- New dependency: nlohmann/json 3.12.0. Client options `--content-dir`, `--map`.
- 79 CTest tests.

### Phase 0 — environment and repository
- CMake (C++20) project with `windows-debug` / `windows-release` presets (MSVC + Ninja).
- Dependencies pinned via FetchContent: SDL3 3.4.18, ENet 1.3.18, spdlog 1.15.3, GoogleTest 1.17.0.
- `shared`: `wind::log` logging interface, CLI helpers, generated version header.
- `server`: fixed-rate tick loop (20 Hz default), ENet initialization, Ctrl+C shutdown, `--max-ticks`.
- `client`: SDL3 window and renderer that clears each frame; `--max-frames`.
- 20 CTest tests, including a server start/stop smoke test.
- Build, test, and run scripts in `scripts/`; build guide in `docs/BUILD.md`.
