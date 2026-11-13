# Completing the live layout

The application is a generic external adapter. It does not discover Standoff 2 structures or translate emulator addresses automatically. Completing this file requires inspecting the specific installed game and BlueStacks runtime. No known-working Standoff 2 addresses are supplied.

Every `null` or empty required collection in the shipped file is a `// FILL:` item. JSON cannot contain comments, so the literal instruction lives in its `_fill` string. Keep `enabled: false` until all required fields are known.

## 1. Identify the exact runtime

Run `tools/capture_identity.ps1` in PowerShell on the Windows machine. It writes `session-identity.json` listing HD-Player process IDs and creation times. Select the process corresponding to the desired BlueStacks instance. Copy its values into `process_id` and `process_created_filetime`.

Using the ADB executable supplied with your BlueStacks installation:

```powershell
& 'C:\Program Files\BlueStacks_nxt\HD-Adb.exe' devices
& 'C:\Program Files\BlueStacks_nxt\HD-Adb.exe' -s '<device-serial>' shell getprop ro.product.cpu.abilist
& 'C:\Program Files\BlueStacks_nxt\HD-Adb.exe' -s '<device-serial>' shell dumpsys package com.axlebolt.standoff2
& 'C:\Program Files\BlueStacks_nxt\HD-Adb.exe' -s '<device-serial>' shell pm path com.axlebolt.standoff2
```

Replace `<device-serial>` with the exact value returned by `devices`. ADB must be enabled in the instance settings. Read `versionName` from package output into `game_version`. Inspect the installed package's native libraries to identify the actual game ABI, which may differ from the guest OS ABI. Read the ELF build ID of that ABI's `libil2cpp.so` with `llvm-readelf -n`; store it in `libil2cpp_build_id`. Use ELF32/ELF64 to set `guest_pointer_bytes` to 4/8. These version strings are provenance labels, not an automatic verification of the running image.

## 2. Capture mappings

`regions` maps **guest process virtual addresses** to **Windows HD-Player virtual addresses**:

```json
{"guest": "0x...", "host": "0x...", "size": "0x..."}
```

These three values must be captured from the running instance's address translation, not inferred by subtracting two module bases. A guest `/proc/<pid>/maps` file alone is insufficient: it contains no Windows backing addresses. Host virtual memory region enumeration alone is also insufficient.

For each required guest page, use the emulator's available debugging/VM instrumentation to resolve guest virtual to guest physical, then guest physical to the Windows backing allocation. Record the resulting guest-virtual/host-virtual page pair. The precise VM inspection interface depends on the installed BlueStacks engine and is not implemented here. Do not substitute a guessed fixed translation offset. If that translation is not available, live mode cannot be completed with this adapter.

Verify each captured region by reading known bytes independently in the guest and host and comparing them. Only merge adjacent pages when both address spaces are contiguous. Include every page needed by roots, pointer chains, arrays and transforms. Regions may be supplied in any order; overlaps, empty spans and overflowing spans are rejected.

`markers` contains at least one object of the form `{"address": "0x...", "bytes": [integer bytes]}`. Use 8–256 independently captured, stable bytes from the mapped game image; markers are checked every sample. Include markers for each image needed by your layout. They do not validate unrelated heap pages. Refresh mappings when pages change and after every emulator restart. The process creation time prevents reusing an old mapping after a restart/PID reuse.

## 3. Recover roots and field paths

Inspect the matching native game image and IL2CPP metadata, if present and readable, with a metadata-aware reverse-engineering tool. Resolve the runtime local-player reference, player collection and camera from that build. Confirm each address in a live debugger by changing a known in-game value and observing the corresponding field. If metadata or fields are encoded, this adapter does not decode them.

Every root has an absolute **guest** `base` and an `offsets` list. All offsets except the final one mean “add offset and read a guest-width pointer”; the final offset locates the value itself. For example `[16, 32]` means `read_guest_pointer(base + 16) + 32`. `[0]` reads directly at the base. Paths contain 1–16 offsets.

| Root | Value at the resolved address |
| --- | --- |
| `matrix` | Combined camera view-projection matrix: sixteen float32 values |
| `local_player` | Guest-width pointer to the local player |
| `entities` | Guest-width pointer to the entity pointer array object |
| `count` | Signed 32-bit number of entries |

Set `array_data_offset` to the pointer storage's displacement from the array object; set `array_stride` to the distance between adjacent pointers. Do not copy IL2CPP array header sizes from another ABI/version. Set `max_players` to a verified bound, at most 128.

The `fields` paths are relative to each player object. Each numeric field now takes an explicit object: `{"offsets": [16, 4], "type": "i16"}`. Supported types are `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, and `bool`. Use the scalar type of the exact declaring field, not the feature name. The supplied dump has several different ammo representations. Old array-only configurations retain their original int32 meaning (uint8 for visibility) for compatibility; they are not auto-corrected. If the installed game uses floats, encoded values, dictionaries, or another collection structure, the adapter must be extended before enabling it. A weapon path can traverse through the equipped-weapon object. Confirm magazine count by firing and reloading; confirm health and armour independently. Populate `weapon_names` from the matching game's ID table. An optional `visible` field can use `bool` (strict 0/1) or another explicitly selected integer type; use a field with the intended visibility meaning, not a guessed flag.

The code ignores null entries and the local-player object. It expects the array's non-null entries to be player objects. A mixed entity registry needs a type-aware adapter first.

## 4. Recover positions and camera conventions

Each of `feet`, `head`, `chest`, `spine`, and `eye` has a relative field path and a `kind`:

- `vec3`: the final address contains an actual world-space float32 XYZ vector.
- `transform`: the final address contains a guest pointer to a Unity transform structure.

For transforms, identify the transform's access-data pointer and hierarchy index; put their paths in `data_path` and `index_path`. Relative to the access-data object, supply `matrices_path` and `parents_path` locating pointers to the transform records and signed int32 parent indices. Set record stride and the translation(float3), rotation(XYZW quaternion), scale(float3) displacements. Set the parent-index stride. The reader composes parent scale, rotation and translation until parent `-1`, rejecting cycles, invalid indices and excessive depth. This supports an uncompressed TRS hierarchy, not every possible Unity animation representation.

Find bone transforms by the actual model skeleton names/indices; verify the points as the player crouches, rotates and animates. Do not use the local camera location as an enemy head location.

Input vectors use Unity-style XYZ with Y up. The adapter normalizes them to its renderer's coordinate system. `world_unit_meters` defaults to 1 and needs calibration against a known distance. `matrix_column_major` describes the layout of the sixteen captured floats. `clip_w_sign` is 1 or -1 depending on which side of the camera has positive clip W. Check center, left/right, up/down and behind-camera points before enabling aim.

## 5. Match the viewport and activate

Set `viewport_insets_px` to `[left, top, right, bottom]` in physical pixels inside the main BlueStacks window's client area. Exclude its toolbars and any black bars. The overlay runs DPI-aware but cannot infer the game's actual render viewport. Insets must be refreshed if the emulator layout changes.

Save the completed JSON with `enabled: true`. The process reloads it once per second or through CONFIG → Reload layout. Test ESP projection first, then enable aim and calibrate mouse gain with the emulator's shooting mode active. Unknown visibility disables targets under the default visibility filter. A root/mapping/marker failure clears the entire frame and stops input; automatic mapping repair is not supplied.
