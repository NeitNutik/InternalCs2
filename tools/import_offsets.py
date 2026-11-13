#!/usr/bin/env python3
"""Import a matched set of a2x/cs2-dumper JSON files. Network access is optional."""
import argparse
import json
import os
from pathlib import Path
from urllib.request import urlopen, Request

FIELDS = {
    "health": [
        "C_BaseEntity",
        "m_iHealth"
    ],
    "max_health": [
        "C_BaseEntity",
        "m_iMaxHealth"
    ],
    "team": [
        "C_BaseEntity",
        "m_iTeamNum"
    ],
    "life_state": [
        "C_BaseEntity",
        "m_lifeState"
    ],
    "scene_node": [
        "C_BaseEntity",
        "m_pGameSceneNode"
    ],
    "origin": [
        "CGameSceneNode",
        "m_vecAbsOrigin"
    ],
    "dormant": [
        "CGameSceneNode",
        "m_bDormant"
    ],
    "pawn_handle": [
        "CCSPlayerController",
        "m_hPlayerPawn"
    ],
    "player_name": [
        "CBasePlayerController",
        "m_iszPlayerName"
    ],
    "armour": [
        "C_CSPlayerPawn",
        "m_ArmorValue"
    ],
    "immunity": [
        "C_CSPlayerPawn",
        "m_bGunGameImmunity"
    ],
    "view_offset": [
        "C_BaseModelEntity",
        "m_vecViewOffset"
    ],
    "weapon_services": [
        "C_BasePlayerPawn",
        "m_pWeaponServices"
    ],
    "active_weapon": [
        "CPlayer_WeaponServices",
        "m_hActiveWeapon"
    ],
    "clip": [
        "C_BasePlayerWeapon",
        "m_iClip1"
    ],
    "attribute_manager": [
        "C_EconEntity",
        "m_AttributeManager"
    ],
    "item": [
        "C_AttributeContainer",
        "m_Item"
    ],
    "item_definition": [
        "C_EconItemView",
        "m_iItemDefinitionIndex"
    ],
    "spotted_state": [
        "C_CSPlayerPawn",
        "m_entitySpottedState"
    ],
    "spotted": [
        "EntitySpottedState_t",
        "m_bSpotted"
    ],
    "spotted_mask": [
        "EntitySpottedState_t",
        "m_bSpottedByMask"
    ],
    "model_state": [
        "CSkeletonInstance",
        "m_modelState"
    ],
    "velocity": [
        "C_BaseEntity",
        "m_vecAbsVelocity"
    ]
}
MODULES = {
    "entity_list": ("client.dll", "dwEntityList"),
    "local_pawn": ("client.dll", "dwLocalPlayerPawn"),
    "local_controller": ("client.dll", "dwLocalPlayerController"),
    "view_matrix": ("client.dll", "dwViewMatrix"),
    "view_angles": ("client.dll", "dwViewAngles"),
    "build_number": ("engine2.dll", "dwBuildNumber"),
}

def fetch(url):
    with urlopen(Request(url, headers={"User-Agent": "toker-offset-importer"}), timeout=30) as response:
        return json.load(response)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--dump-dir", type=Path, help="Directory with offsets.json, client_dll.json, info.json")
    source.add_argument("--latest", action="store_true", help="Resolve main to one commit, then download all files from it")
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parents[1] / "config" / "offsets.json")
    args = parser.parse_args()
    current = json.loads(args.output.read_text(encoding="utf-8"))
    if args.latest:
        commit = fetch("https://api.github.com/repos/a2x/cs2-dumper/commits/main")["sha"]
        def read(name):
            return fetch(f"https://raw.githubusercontent.com/a2x/cs2-dumper/{commit}/output/{name}")
    else:
        commit = "local-dump"
        def read(name):
            return json.loads((args.dump_dir / name).read_text(encoding="utf-8"))
    offsets, schema, info = (read(name) for name in ("offsets.json", "client_dll.json", "info.json"))
    classes = schema["client.dll"]["classes"]
    current["modules"] = {key: offsets[module][field] for key, (module, field) in MODULES.items()}
    current["fields"] = {key: classes[cls]["fields"][field] for key, (cls, field) in FIELDS.items()}
    current["build_number"] = int(info["build_number"])
    current["source_timestamp"] = info["timestamp"]
    current["source_commit"] = commit
    for group in ("modules", "fields"):
        for key, value in current[group].items():
            if not isinstance(value, int) or isinstance(value, bool) or value < 0:
                raise ValueError(f"Invalid {group}.{key}: {value!r}")
    temporary = args.output.with_suffix(".json.tmp")
    temporary.write_text(json.dumps(current, indent=2) + "\n", encoding="utf-8")
    os.replace(temporary, args.output)
    print(f"Imported build {current['build_number']} from {commit}")
    print("Layout constants were preserved. The dumper does not supply entity stride or the bone array pointer offset.")

if __name__ == "__main__":
    main()

