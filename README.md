# TOKER / Standoff 2 / BlueStacks 5

External Windows x64 C++20/ImGui project with a standalone synthetic preview and a configurable emulator-memory adapter.

**Status: compiled prototype, not a ready-to-run Standoff 2 cheat.** No live BlueStacks installation, game dump, or verified guest-to-host mappings were available. `config/layout.json` deliberately ships disabled, with unknown addresses marked `FILL`. Live ESP and aim require completing that layout. Silent aim is not implemented.

## What is included

| Feature | Implementation |
| --- | --- |
| ESP rendering | Full/corner boxes, straight snaplines, curved spline snaplines, distance, health/armour bars, weapon label, magazine count |
| Target selection | Head/chest/spine; circular screen FOV in pixels; team, range, health, immunity, and optional visibility filters |
| Aim input | Hold Alt, left mouse, right mouse, or a custom key; smoothing, mouse gain, capped steps; pauses for menu/focus loss/invalid frames |
| Radar | Draggable, view-relative overlay; does not alter the game's built-in radar |
| Menu | VISUALS, AIM, MOVEMENT, MISC, CONFIG; JSON save/load with backups |
| Memory adapter | Read-only Windows process handle; 32/64-bit guest pointers; explicit mappings; pointer chains; world vectors or Unity TRS hierarchy positions |
| Session checks | Process name, PID, creation time, memory markers, bounded collections, matrix/position checks; incomplete config disables live mode |
| Preview | Synthetic players exercising the same visual renderer, target selector and mouse-step calculation |

The reader opens `HD-Player.exe` with `PROCESS_VM_READ` and query access. Mouse input uses `SendInput`. There is no DLL injection, Present hook, driver, anti-cheat evasion, recoil modification, trigger firing, or shot-command patch.

**BlueStacks can move guest pages without changing the Windows PID; a valid process handle alone does not make an old guest mapping valid.** Process timestamps and markers reduce obvious stale-session errors but cannot prove every mapping remains current. Mapping capture is not automated by this project.

## Run

The resize-fix build releases the DirectX context's back-buffer reference before resizing. It also skips redundant window resizes. Graphics failures display their HRESULT and append details to `config/graphics-error.log` instead of silently exiting. This addresses a confirmed resource-lifetime error in the overlay; it does not establish the cause of a crash inside Standoff 2 or BlueStacks.

Extract the archive on Windows 10/11 x64 and keep `bin/config` beside the executables.

1. Run `bin/toker_preview.exe` to inspect the menu and synthetic scene immediately.
2. In AIM, select a target point and key, enable aim, and close the menu with Insert. Arrow keys turn the preview camera; R resets it. Mouse gain changes the simulated input response.
3. For live mode, follow [docs/LAYOUT.md](docs/LAYOUT.md), fill `bin/config/layout.json`, then run `bin/toker_standoff2.exe`.
4. Insert opens/closes the menu. End exits the live overlay. Both keys are reserved from aim binding.
5. The live overlay follows the configured BlueStacks window and hides when it is minimized or another application has focus. Set the viewport insets to exclude emulator toolbars and letterboxing.

Release BlueStacks mouse capture before using the menu, and enable its shooting/mouse-look mode before using aim. Bindings vary by emulator profile; this project does not change that profile. Whether BlueStacks accepts the injected relative mouse movement must be checked on the installed version. Windows input integrity levels can also reject `SendInput`; the code reports failure.

Aim defaults to disabled. Visibility filtering defaults to enabled. If `fields.visible` is absent, no entity passes that filter; the menu lets you turn it off explicitly. The generic reader has no immunity field or raycast implementation, so it does not identify spawn immunity or prove line of sight. Labels use `Player N`; weapon IDs use the supplied `weapon_names` table, with `Weapon #ID` as a factual fallback.

## Silent aim

Normal aim changes the camera through mouse input. Silent aim would require verified shot/command data that can differ from that camera. Sending mouse movement cannot achieve this. A client-side trace also cannot force the server to register a headshot. No silent-aim toggle or fake implementation is included.

## Build

Dependencies are included: Dear ImGui 1.91.8 and nlohmann/json 3.11.3, with their original licenses. No dependency download is required.

Windows: Visual Studio 2022, Desktop development with C++, Windows SDK, CMake 3.24+.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Executables and their `config` directory appear in `build/Release`.

Linux core tests:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Linux Windows cross-build:

```sh
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/llvm-mingw.cmake -DLLVM_MINGW_ROOT=/path/to/llvm-mingw -DCMAKE_BUILD_TYPE=Release
cmake --build build-win --parallel
```

See [VALIDATION.md](VALIDATION.md) for the checks performed and their limits.
