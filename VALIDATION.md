# Validation — 2026-11-13

Passed:

- Portable C++20 Debug build: GCC 14.2.0, CMake 4.4.4.
- CTest: one test executable, 34 checks, no failures.
- Windows x64 Release cross-build: llvm-mingw 20250114 / Clang 19.1.7.
- Targets built: `toker_standoff2.exe`, `toker_preview.exe`, `toker_tests.exe`.
- PE import inspection: Windows/UCRT/D3D dependencies only; no separately distributed C++ runtime or ImGui DLL.

Tests cover guest pointer sizes, address overflow, pointer chains, overlapping mappings, cross-page reads, unmapped gaps, Unity parent transforms, hierarchy cycles/indices, coordinate and matrix normalization, behind-camera rejection, pixel FOV, bone/team/life/immunity/visibility/range filters, inactive-frame rejection, mouse-step bounds, radar orientation, JSON persistence, reserved hotkeys, complete synthetic layouts and rejection of incomplete/invalid layouts.

Not tested: native MSVC compilation, Windows GUI execution, DPI/window tracking, BlueStacks mouse-input acceptance, any Standoff 2 runtime address, actual game projection, or live aim. No Windows/BlueStacks session or game dump was available.

The packaged live layout is deliberately disabled and incomplete. The fixture used in tests is synthetic and must never be used as a game layout. The adapter only supports a host where the configured guest pages reside inside `HD-Player.exe`; a BlueStacks engine placing them in a different host process needs a different adapter. It does not discover mappings or game structures.

Silent aim and anti-cheat evasion are not implemented. Passing these checks establishes portable-logic correctness for the covered cases and successful cross-compilation, not a working live-game cheat.
