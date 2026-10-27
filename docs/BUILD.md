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

`build.ps1` loads the MSVC x64 developer environment (`scripts\dev_environment.ps1`), then runs the CMake presets. The first configure clones SDL3, ENet, spdlog, and GoogleTest (≈40 s); the first full build compiles about 320 steps.

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

The run on 2026-10-27 passed 20/20 tests: 17 GoogleTest cases plus a process smoke test that starts `wind_server` for 5 ticks and checks for a clean exit.

## 4. Run

```powershell
.\scripts\run_server.ps1                    # Ctrl+C to stop
.\scripts\run_server.ps1 --log-level debug --tick-rate 30
.\scripts\run_client.ps1                    # close the window to quit
.\scripts\run_client.ps1 --width 1600 --height 900
```

Pass `-Preset windows-release` to either script to run the Release build. Each executable lists its options with `--help`.

Smoke-test flags (used for verification, not gameplay):

- `wind_server --max-ticks N` stops after N ticks.
- `wind_client --max-frames N` closes the window after N frames.

## Troubleshooting

- **`cl.exe` not found / Ninja picks the wrong compiler:** run the build from `scripts\build.ps1`, or dot-source `scripts\dev_environment.ps1` first.
- **First configure fails while cloning:** FetchContent needs network access to github.com.
- **Changing a dependency version:** edit `cmake/Dependencies.cmake`, update `DEPENDENCIES.md`, then delete `build\<preset>` and reconfigure.
