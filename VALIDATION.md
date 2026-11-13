# Validation — 2026-11-13

## Passed

- Native Linux C++20 Debug build with GCC 14.2.0 and CMake 4.4.4.
- CTest portable suite: 1 executable, 32 checks, 0 failures.
- Windows x64 Release cross-build with llvm-mingw 20250114 / Clang 19.1.7: `toker_cs2.dll`, `toker_preview.exe`, `toker_tests.exe`.
- Rebuilt both targets using the included `vendor` source tree.
- Imported the pinned a2x dump from local JSON and compared every module offset, schema field, preserved layout value, and build number with the packaged configuration.
- Inspected PE imports: no separate MinHook, ImGui, JSON, libc++, or libunwind DLL is required. Windows system/UCRT/D3D libraries are required.

The portable checks cover camera projection, invalid coordinates, yaw wrapping, frame-independent smoothing, radar orientation/range, aim FOV/team/spotted/immunity/life/range/bone selection, nearest-angle selection, JSON round trips/backups, transactional rejection of invalid settings, and malformed/unsafe offsets.

## Not tested

- Native MSVC compilation.
- Executing the Windows GUI or DLL.
- Hook initialization, resize behavior, mouse capture, or device recovery in CS2.
- Runtime offsets, bone layouts, and actual aim behavior in CS2 build 14197.
- VAC behavior or any undetectability property.

No game process or Windows runtime was available. A successful cross-build and passing portable tests do not establish live-game compatibility. VAC bypass and silent aim are not implemented.
