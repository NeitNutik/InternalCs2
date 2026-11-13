# TOKER / kindmops

C++20 project for Windows x64: a DX11 in-process DLL, Dear ImGui menu, and standalone preview application. This is an unverified game integration, with compiled binaries and tested portable logic.

## Included

| Feature | Implementation |
| --- | --- |
| ESP | Full/corner boxes, straight/curved snaplines, distance, names, health/armour bars, weapon name, magazine count |
| Aim | Hold Alt, left mouse, right mouse, or a custom key; head/chest/spine selection; angular FOV; time-based smoothing; distance, team, immunity, and spotted-state filters |
| Radar | Draggable, view-relative ImGui radar using sampled player positions |
| Menu | VISUALS, AIM, MOVEMENT, MISC, CONFIG; Insert toggles the window |
| Configuration | JSON settings, backup on save, transactional load, automatic offset-file reload |
| Hook | Standard MinHook detours on DX11 Present and ResizeBuffers; Win32 input subclass |
| Preview | Animated synthetic players, same ESP/menu/aim logic, no game required |

**Not implemented:** VAC bypass, undetectability, silent aim, modification of the game's built-in radar, DX12/Vulkan rendering. The MOVEMENT tab provides speed telemetry, not movement modification. The spotted-state filter is not a line-of-sight trace.

Silent aim changes command/subtick shot angles separately from the displayed camera. It requires verified command layouts and a valid hook in command generation. A local bullet trace cannot force a server-authoritative hit. This project only writes ordinary view angles and does not present that as silent aim.

The Present detour modifies executable code inside the process; this is observable and is not a VAC bypass.

## Quick preview

Run `bin/toker_preview.exe` on Windows 10/11 x64. Keep the adjacent `bin/config` directory.

- Insert: open/close the menu.
- AIM: enable aim, select a key and target point, then close the menu and hold the key.
- Arrow keys: turn the preview camera while the menu is closed. R: reset it.
- CONFIG: save/load `settings.json` or restore defaults.
- Radar: drag its title while the menu is open.

Aim is disabled initially and pauses whenever the menu is open or the application loses focus. The default FOV radius is 5 degrees. The preview renders synthetic data; it does not establish compatibility with a running CS2 build.

## Build

Install Visual Studio 2022 with Desktop development with C++ and the Windows SDK, plus CMake 3.24 or newer. Dependencies are included in `vendor`; normal builds do not need network access.

From the project root in a developer terminal:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Outputs: `build/Release/toker_cs2.dll`, `toker_preview.exe`, and `config/`.

Linux portable checks:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Linux cross-build with llvm-mingw:

```sh
cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/llvm-mingw.cmake -DLLVM_MINGW_ROOT=/path/to/llvm-mingw -DCMAKE_BUILD_TYPE=Release
cmake --build build-win --parallel
```

## DLL integration

The DLL expects the 64-bit Windows DX11 process and normal Windows loader initialization. Load it with an existing x64 development loader that calls `LoadLibraryW`; a separate injector is not included. `DllMain` launches initialization on a worker thread. Settings and offsets are located relative to the DLL, not the current working directory.

Open the game console or Escape menu first if the game's relative mouse mode keeps recentering the pointer. This project subclasses Win32 input messages but does not hook the engine's relative-mouse input subsystem.

The DLL stays resident until process exit. Manual unloading is unsupported: Present and the window procedure retain callback addresses inside it. The standard DX11 swap-chain path is implemented; replacement of the swap-chain object or device removal has not been validated.

All runtime reads use `ReadProcessMemory(GetCurrentProcess(), ...)` from inside the DLL, so a stale address returns an error rather than being dereferenced directly. The only game write is the three-component view angle value. This has syscall overhead and samples on each rendered frame. There is no external process reader, driver, concealment, or command buffer manipulation.

## Offset data

The bundled JSON is based on `a2x/cs2-dumper` commit `cb01076744f3554324d76b7a4830f0fcdfde95fb`, build **14197**, dump timestamp `2026-11-12T13:07:45.363725600+00:00`.

The loader rejects malformed offsets. The game adapter checks module image ranges and requires the engine build number to match before reading player fields or writing angles. It also validates the local controller-to-pawn mapping, local team/life/health, and bone positions. A mismatch produces a menu status instead of enabling aim.

Update from one pinned upstream revision:

```powershell
python tools/import_offsets.py --latest
```

Or import a matched local dump:

```powershell
python tools/import_offsets.py --dump-dir C:\path\to\cs2-dumper\output
```

The source `config/offsets.json` is updated. Copy it beside the DLL into its `config` directory, or use `--output bin/config/offsets.json` to update the prebuilt distribution directly. The running DLL detects file changes once per second. A malformed replacement disables game access until corrected.

`modules` and `fields` come from the dump. `layout` contains separately maintained assumptions: entity chunk table offset 16, entry stride 112, 512 entries per chunk, handle index mask 32767, bone array pointer at model state +128, bone stride 32, and head/chest/spine indices 6/4/2. These values have not been checked against a live local game. The importer preserves them because schema output does not contain them.

If layout validation fails after an update:

1. Dump the installed build and import its `offsets.json`, `client_dll.json`, and `info.json` together.
2. Inspect the entity lookup routine in `client.dll`. Record the chunk-table displacement, index shift/mask, and entry-size multiplier in `layout`.
3. At the local controller, confirm that resolving `m_hPlayerPawn` returns `dwLocalPlayerPawn`.
4. Inspect the scene node's model state and its bone-transform array. Record the array-pointer displacement and transform stride; confirm head/chest/spine indices against the model skeleton.
5. Replace the JSON. Do not change the recorded build number merely to silence the mismatch status.

## Validation

See `VALIDATION.md` for the exact build and test results. No live CS2 session, VAC behavior, game hook execution, or Windows GUI execution was tested in this Linux environment.

## Dependencies

- Dear ImGui 1.91.8, commit `dbb5eeaadffb6a3ba6a60de1290312e5802dba5a`, MIT.
- MinHook 1.3.4, commit `c3fcafdc10146beb5919319d0683e44e3c30d537`, BSD 2-clause plus bundled HDE notices.
- nlohmann/json 3.11.3, commit `9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03`, MIT.
- CS2 schema/offset data from a2x/cs2-dumper, MIT.

Original license files are retained in `vendor`. JSON is vendored as its unmodified single header with a small local CMake interface target.
