# Findings from the supplied dump

Source: `dump.cs` extracted from the supplied `dump.zip`, 45,836,699 bytes.

SHA256: `73da8276c235f75f5ab8272246e488e4fdc541b2fc9fed1ef8fdc9eee409e8c9`.

The original dump is not bundled or uploaded. `config/dump-catalogue.json` contains the selected declarations, source line numbers, declaring types and offsets. The importer reads text only. It does not execute anything in the input, treat instructions as commands, or use method RVAs as field offsets.

## Verified declarations

| Declaring type | TypeDefIndex | Field | Relative offset | Scalar type |
| --- | ---: | --- | ---: | --- |
| SpectatePlayerGeneralInfo | 10528 | Team | 0x10 | byte / u8 |
| SpectatePlayerGeneralInfo | 10528 | Hp | 0x12 | ushort / u16 |
| SpectatePlayerGeneralInfo | 10528 | MaxHp | 0x14 | ushort / u16 |
| SpectatePlayerGeneralInfo | 10528 | Armor | 0x16 | ushort / u16 |
| object.AmmoData | 10609 | Ammo backing field | 0x8 | byte / u8 |
| object.AmmoData | 10609 | RestAmmo backing field | 0xA | ushort / u16 |
| WeaponSnapshot | 14831 | Id backing field | 0x0 | byte / u8 |
| WeaponSnapshot | 14831 | Ammo backing field | 0x4 | short / i16 |
| WeaponSnapshot | 14831 | RestAmmo backing field | 0x6 | short / i16 |
| WeaponId | 34725 | Raw backing field | 0x0 | byte / u8 |

These are offsets **within those exact values**. `SpectatePlayerGeneralInfo` is HUD/spectator data, not an `odq` player object. A managed struct uses its unboxed value base; a boxed value or a containing class has a different starting address. Copying `0x12` to an arbitrary player pointer is incorrect.

The old reader's unconditional int32 reads could merge adjacent packed fields. For instance, reading four bytes where `Hp` and `MaxHp` are adjacent uint16 values combines them into a different integer. The reader now handles explicit scalar widths, and regression tests cover adjacent values, page boundaries and signed ammo sentinels. This was a concrete compatibility problem in the generic adapter, not proof of the cause of the reported game crash.

## Player and bone evidence

- `odq` (14775) has a `ydm` reference named `djka` at `+0x70`. `ydm` (14793) declares six adjacent ushort fields at `+0x10` through `+0x1A`. Their names are obfuscated; the dump alone does not establish which is current HP, maximum HP or armour. None has been assigned a guessed meaning in the active configuration.
- `odq` also has a `float3[]` reference at `+0x60`. Its contents and bone ordering are not declared. It is not assumed to be a head/spine array.
- `CharacterBindings` (35670) has `_bones` at `+0x20`, of type `BonesDictionary`. The `BonesDictionary` declaration has no fields in this dump. Its native backing layout cannot be recovered from this declaration alone.
- `HumanBodyBones` (21125) defines `Spine = 7`, `Chest = 8`, `Head = 10`. These are enum keys, not proven indices into an arbitrary native transform array.
- References spaced eight bytes apart are consistent with a 64-bit managed object layout. The dump is not an ELF header and does not prove which CPU ISA or BlueStacks ABI produced it.

## Silent aim evidence

`HitCastData` (20274) declares `ShotPoint` at `+0x10` and `Direction` at `+0x1C`. That establishes the shape of a value type; it does not locate a live shot, establish its lifetime, or show whether it is an input or already-processed result. Writing to a guessed address would not implement reliable silent aim. This project has no shot-command write path and does not expose a working silent-aim switch.

## Reproduce extraction

Requires Python 3.10 or newer:

```powershell
python tools/import_dump.py C:\path\to\dump.cs
```

The default selection targets the supplied dump and validates type names at the selected TypeDefIndex values. For a different dump, choose its indices explicitly with repeated `--type-index` arguments. The output includes 19 selected types and 38 relative scalar bindings for this file. Static fields are listed as static and excluded from instance bindings. Enums are recorded separately.

No active field is automatically replaced. A relative binding such as `14831:<Ammo>k__BackingField` says that an already-resolved **WeaponSnapshot value** has an i16 at `+0x4`. A verified path from each player to that snapshot is still required.

## What is still missing

To complete live integration, the following must come from the installed runtime, rather than this textual dump:

1. The matching `libil2cpp.so`, its ELF build ID and process load mappings. Native disassembly can establish which obfuscated property getters are used for HP/armour and which concrete objects store the collections. The dump's method bodies are empty.
2. The live local-player and player-collection roots, concrete collection representation and element types.
3. Guest-virtual to Windows-host address mappings for the active BlueStacks instance. Android `/proc/<pid>/maps` alone does not provide host mappings.
4. The active camera matrix location and native transform/character hierarchy layout.

Runtime/session fields remain disabled in `layout.json` because the attachment cannot supply them. Their capture procedure and the adapter's supported representations are described in `LAYOUT.md`. The scalar fix, catalogue, menu and preview are implemented; a verified live Standoff 2 adapter is still incomplete.
