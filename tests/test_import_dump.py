import importlib.util
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location("import_dump", Path(__file__).parents[1] / "tools" / "import_dump.py")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
FIXTURE = """// Image 0: Example.dll - 0
// Namespace: Example
public struct Packed : object // TypeDefIndex: 1
{
    // Fields
    public ushort Hp; // 0x0
    private short <Ammo>k__BackingField; // 0x2
    public static int Counter; // 0x0
    // Properties
    public ushort Health { get; }
    // Methods
    // RVA: 0x123400 Offset: 0x123000 VA: 0x123400
    public void Update() { }
}
// Namespace: Other
internal class Packed // TypeDefIndex: 2
{
    // Fields
    public byte Team; // 0x10
}
// Namespace: Example
private enum Bones // TypeDefIndex: 3
{
    // Fields
    public int value__; // 0x0
        Spine = 7;
        Head = 10;
}
"""

class ImportTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.file = Path(self.directory.name) / "dump.cs"
        self.file.write_text(FIXTURE, encoding="utf-8")
    def tearDown(self):
        self.directory.cleanup()
    def test_widths_and_struct_base(self):
        result = MODULE.catalogue(self.file, [1])
        hp = result["relative_field_bindings"]["1:Hp"]
        self.assertEqual((hp["offsets"], hp["type"], hp["requires_unboxed_struct_base"]), ([0], "u16", True))
        self.assertEqual(result["relative_field_bindings"]["1:<Ammo>k__BackingField"]["type"], "i16")
    def test_static_not_live_binding(self):
        result = MODULE.catalogue(self.file, [1])
        self.assertNotIn("1:Counter", result["relative_field_bindings"])
        self.assertEqual(result["types"][0]["fields"][2]["storage"], "static")
    def test_method_rva_not_field(self):
        result = MODULE.catalogue(self.file, [1])
        self.assertEqual(len(result["types"][0]["fields"]), 3)
        self.assertNotIn("123400", str(result["types"]))
    def test_same_names_are_separate(self):
        result = MODULE.catalogue(self.file, [1, 2])
        self.assertEqual([t["namespace"] for t in result["types"]], ["Example", "Other"])
        self.assertFalse(result["relative_field_bindings"]["2:Team"]["requires_unboxed_struct_base"])
    def test_enum(self):
        result = MODULE.catalogue(self.file, [3])
        self.assertEqual(result["types"][0]["enum_values"], {"Spine": 7, "Head": 10})
        self.assertEqual(result["relative_field_bindings"], {})
    def test_missing_type(self):
        with self.assertRaises(ValueError):
            MODULE.catalogue(self.file, [999])
    def test_duplicate_index(self):
        self.file.write_text(FIXTURE + "\n// Namespace: X\npublic struct X // TypeDefIndex: 1\n{}\n")
        with self.assertRaises(ValueError):
            MODULE.catalogue(self.file, [1])
    def test_source_hash_and_nonactivation(self):
        result = MODULE.catalogue(self.file, [1])
        self.assertEqual(result["source_sha256"], MODULE.hashlib.sha256(self.file.read_bytes()).hexdigest())
        self.assertFalse(result["runtime_verified"])
        self.assertEqual(list(Path(self.directory.name).iterdir()), [self.file])

if __name__ == "__main__":
    unittest.main()
