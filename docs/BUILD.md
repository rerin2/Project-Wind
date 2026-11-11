# Build Guide (Windows)

Every command here was run on Windows 11 Pro (10.0.26200) on 2026-10-27, with the results noted.

## 1. Install the toolchain

These are the `winget` commands used to set up the verified environment:

```powershell
winget install --id Kitware.CMake -e
winget install --id Ninja-build.Ninja -e
winget install --id Git.Git -e
winget install --id Microsoft.VisualStudio.2022.BuildTools -e --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

Open a **new** terminal afterwards so `cmake`, `ninja`, and `git` are on `PATH`.

| Tool | Verified version |
|---|---|
| MSVC (VS 2022 Build Tools) | 19.44.35229 x64 |
| CMake | 4.4.4 (presets need ≥ 3.25) |
| Ninja | 1.13.2 |
| Git | 2.56.0 |

If Windows blocks the `.ps1` scripts, allow local scripts for your user once:
`Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`.

## 2. Build

```powershell
.\scripts\build.ps1                         # Debug  -> build\windows-debug
.\scripts\build.ps1 -Preset windows-release # Release -> build\windows-release
```

`build.ps1` loads the MSVC x64 developer environment (`scripts\dev_environment.ps1`), then runs the CMake presets. The first configure clones SDL3, SDL3_ttf (with its FreeType submodule), spdlog, nlohmann/json, and GoogleTest.

Doing the same by hand, from any PowerShell:

```powershell
. .\scripts\dev_environment.ps1     # dot-source: puts cl.exe on PATH
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Executables are written to `build\<preset>\bin\`.

## 3. Test

```powershell
.\scripts\run_tests.ps1             # = ctest --preset windows-debug
```

Like `build.ps1`, `run_tests.ps1` loads the MSVC developer environment first, so it also works on a PC where `ctest` comes only with Visual Studio and is not on `PATH`. When running `ctest` by hand, dot-source `scripts\dev_environment.ps1` first.

The run on 2026-11-10 passed 182/182 tests: GoogleTest cases for the shared and client modules, including validation of the repository's real test map and sprite sheet.

## 4. Run

```powershell
.\scripts\run_client.ps1                    # close the window to quit
.\scripts\run_client.ps1 --width 1600 --height 900
```

Pass `-Preset windows-release` to run the Release build. The client lists its options with `--help`.

Controls: arrow keys / WASD move, Shift (held) run, Space (or K / X) dodge, F3 debug overlay (hidden at start), F4 collision overlay, F5 skip 3 hours, F6 next weather, F7 UI test panel (↑/↓ choose, Enter press, Esc close), F12 screenshot, Esc quit.

Screenshots go to `%APPDATA%\ProjectWIND\screenshots` (640×360 BMP). For automated checks, save frames without pressing keys:

```powershell
.\scripts\run_client.ps1 --screenshot-at 60 --max-frames 65 --screenshot-dir .\shots
```

The client reads content from the repository's `content/` folder by default (path baked in at build time). To use another copy or map:

```powershell
.\scripts\run_client.ps1 --content-dir D:\wind\content --map maps/test_meadow.json
```

## Regenerating test content

The placeholder art and test map are produced by a script (original shapes, no third-party assets):

```powershell
powershell -NoProfile -File tools\test_content\generate_test_content.ps1
```

It rewrites `content/tilesets/test_tiles.bmp`, `content/ui/ui_frames.bmp`, `content/sprites/test_hero.bmp` and `content/maps/test_meadow.json`, `test_cave.json`, `test_forest.json`. The tileset, sprite and UI theme JSON files are hand-edited and not touched. Re-run the tests afterwards — `MapLoader.RepositoryTestMapIsValid` validates the generated map.

Smoke-test flags (used for verification, not gameplay):

- `wind_client --max-frames N` closes the window after N frames.
- `wind_client --screenshot-at N` saves a screenshot after frame N (repeatable); `--screenshot-dir DIR` picks the folder.
- `wind_client --fx-stress N` keeps about N extra particles on screen (0–4000) to check particle performance on the F3 panel.
- `wind_client --light-stress N` adds N lights (0–64) around the start of a lit map, e.g. `--map maps/test_cave.json --light-stress 32`.
- `wind_client --press f6@60` taps a key at a frame (repeatable; keys f3 f4 f5 f6 f7 f12 space shift enter esc up down left right), e.g. to check F5/F6 without pressing keys.
- `wind_client --hold shift@20-290` holds a key from frame 20 to 290 (repeatable, same key names), e.g. `--auto-move right --hold shift@20-290` runs until exhausted.
- `wind_client --time H --weather ID` starts at hour H (0 to below 24) with weather ID from the map table, e.g. `--time 18.5 --weather rain`.
- `wind_client --auto-move left` holds Left for the whole run; from the meadow start the hero walks into the cave mouth and back (map travel check without pressing keys).

## Troubleshooting

- **`cl.exe` not found / Ninja picks the wrong compiler:** run the build from `scripts\build.ps1`, or dot-source `scripts\dev_environment.ps1` first.
- **First configure fails while cloning:** FetchContent needs network access to github.com.
- **Changing a dependency version:** edit `cmake/Dependencies.cmake`, update `DEPENDENCIES.md`, then delete `build\<preset>` and reconfigure.
