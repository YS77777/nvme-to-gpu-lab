from __future__ import annotations

import importlib.util
import json
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "bin" / "nvme_gpu_lab"
GENERATOR_PATH = ROOT / "scripts" / "generate_file.py"

SPEC = importlib.util.spec_from_file_location("generate_file", GENERATOR_PATH)
assert SPEC is not None and SPEC.loader is not None
GENERATOR = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GENERATOR)


class PatternTests(unittest.TestCase):
    def test_pattern_continues_across_chunks(self) -> None:
        whole = GENERATOR.pattern_block(0, 1024)
        split = GENERATOR.pattern_block(0, 333) + GENERATOR.pattern_block(333, 691)
        self.assertEqual(whole, split)

    def test_size_parser(self) -> None:
        self.assertEqual(GENERATOR.parse_size("64K"), 65536)
        self.assertEqual(GENERATOR.parse_size("2MiB"), 2 * 1024 * 1024)


class BinarySmokeTests(unittest.TestCase):
    def test_reads_partial_final_chunk_and_verifies_pattern(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "pattern.bin"
            size = 4 * 1024 * 1024 + 13
            GENERATOR.generate(path, size=size, chunk_size=700_001)
            completed = subprocess.run(
                [
                    str(BINARY),
                    "--file",
                    str(path),
                    "--block-size",
                    "64KiB",
                    "--verify-pattern",
                    "--json",
                ],
                check=True,
                text=True,
                capture_output=True,
            )
            result = json.loads(completed.stdout)
            self.assertEqual(result["bytes_read"], size)
            self.assertEqual(result["chunks"], 65)
            self.assertTrue(result["verification_ok"])
            self.assertEqual(result["pattern_mismatches"], 0)

    def test_reports_missing_file(self) -> None:
        completed = subprocess.run(
            [str(BINARY), "--file", "does-not-exist.bin"],
            check=False,
            text=True,
            capture_output=True,
        )
        self.assertEqual(completed.returncode, 1)
        self.assertIn("failed to open file", completed.stderr)


if __name__ == "__main__":
    unittest.main()
