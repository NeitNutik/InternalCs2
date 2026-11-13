#!/usr/bin/env python3
"""Extract IL2CPP field declarations without executing dump text or activating live mode."""
import argparse
import bisect
import hashlib
import json
from pathlib import Path
import re

DEFAULT_TYPES = {
    10528: "SpectatePlayerGeneralInfo", 10609: "object.AmmoData",
    14775: "odq", 14793: "ydm", 14831: "WeaponSnapshot",
    15155: "xiz", 15514: "wwv", 20191: "FireHitCastState",
    20196: "HitParameters", 20274: "HitCastData", 20275: "NavigationContext",
    20285: "WeaponContext", 20286: "jrx", 21125: "HumanBodyBones",
    34725: "WeaponId", 35544: "HitBox", 35547: "HitRequest",
    35670: "CharacterBindings", 35671: "BonesDictionary",
}
SCALARS = {"sbyte": "i8", "byte": "u8", "short": "i16", "ushort": "u16",
           "int": "i32", "uint": "u32", "bool": "bool"}
TYPE = re.compile(r"^(?:public|private|internal|protected)\s+(?:(?:sealed|abstract|static|readonly)\s+)*(class|struct|enum|interface)\s+(.+?)\s*(?::\s*(.*?))?\s*// TypeDefIndex:\s*(\d+)\s*$")
FIELD = re.compile(r"^\s*(.+?);\s*//\s*0x([0-9a-fA-F]+)\s*$")
MODIFIERS = {"public", "private", "internal", "protected", "static", "readonly", "const", "volatile"}

def parse_dump(file, requested):
    records, images = {}, []
    namespace, current, in_fields = "", None, False
    with Path(file).open(encoding="utf-8-sig", errors="strict") as stream:
        for line_number, line in enumerate(stream, 1):
            line = line.rstrip("\r\n")
            image = re.match(r"^// Image \d+: (.+) - (\d+)$", line)
            if image:
                images.append((int(image[2]), image[1]))
            if line.startswith("// Namespace: "):
                namespace = line.removeprefix("// Namespace: ")
            matched = TYPE.match(line)
            if matched:
                index = int(matched[4])
                current, in_fields = None, False
                if index in requested:
                    if index in records:
                        raise ValueError(f"Duplicate TypeDefIndex {index}")
                    current = {"type_index": index, "name": matched[2].strip(), "namespace": namespace,
                               "kind": matched[1], "base_declaration": matched[3], "line": line_number,
                               "fields": [], "enum_values": {}}
                    records[index] = current
                continue
            if current is None:
                continue
            stripped = line.strip()
            if stripped == "// Fields":
                in_fields = True
                continue
            if stripped in ("// Properties", "// Methods", "// Events", "}"):
                in_fields = False
            if not in_fields:
                continue
            field = FIELD.match(line)
            if field:
                tokens, modifiers = field[1].split(), []
                while tokens and tokens[0] in MODIFIERS:
                    modifiers.append(tokens.pop(0))
                if len(tokens) < 2 or "=" in tokens:
                    continue
                current["fields"].append({"name": tokens[-1], "type": " ".join(tokens[:-1]),
                    "offset": int(field[2], 16), "storage": "static" if "static" in modifiers else "instance",
                    "line": line_number})
            elif current["kind"] == "enum":
                enum = re.match(r"^\s*(\w+)\s*=\s*(-?(?:0x[0-9a-fA-F]+|\d+));$", line)
                if enum:
                    current["enum_values"][enum[1]] = int(enum[2], 16 if "0x" in enum[2] else 10)
    images.sort()
    for record in records.values():
        i = bisect.bisect_right([start for start, _ in images], record["type_index"]) - 1
        record["assembly"] = images[i][1] if i >= 0 else None
    missing = set(requested) - records.keys()
    if missing:
        raise ValueError(f"Missing TypeDefIndex values: {sorted(missing)}")
    return records

def catalogue(file, requested=None):
    selected = DEFAULT_TYPES if requested is None else {index: None for index in requested}
    records = parse_dump(file, selected)
    for index, expected in selected.items():
        if expected and records[index]["name"] != expected:
            raise ValueError(f"Type index {index} changed: expected {expected!r}, found {records[index]['name']!r}; select indices explicitly for this dump")
    relative = {}
    for index, record in records.items():
        for field in record["fields"]:
            if field["storage"] == "instance" and field["type"] in SCALARS and record["kind"] != "enum":
                relative[f"{index}:{field['name']}"] = {
                    "base_type_index": index, "base_type": record["name"],
                    "requires_unboxed_struct_base": record["kind"] == "struct",
                    "offsets": [field["offset"]], "type": SCALARS[field["type"]],
                    "source_line": field["line"]}
    digest = hashlib.sha256()
    with Path(file).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return {"catalogue_version": 1, "source_sha256": digest.hexdigest(),
            "runtime_verified": False,
            "notice": "Relative field facts only. Not a live layout. No guest mappings, roots, native transform layouts, game version or ELF build ID can be inferred from this catalogue.",
            "types": [records[index] for index in sorted(records)], "relative_field_bindings": relative}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dump", type=Path)
    parser.add_argument("--output", type=Path, default=Path(__file__).resolve().parents[1] / "config" / "dump-catalogue.json")
    parser.add_argument("--type-index", type=int, action="append")
    args = parser.parse_args()
    result = catalogue(args.dump, args.type_index)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    temporary = args.output.with_suffix(args.output.suffix + ".tmp")
    temporary.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    temporary.replace(args.output)
    print(f"Extracted {len(result['types'])} types and {len(result['relative_field_bindings'])} relative scalar bindings")
    print(f"Source SHA256: {result['source_sha256']}")
    print("Live layout was not modified.")

if __name__ == "__main__":
    main()
