# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def load_module(name: str, path: Path):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


resource = load_module(
    "paper1_resource", ROOT / "publication/paper1/run_m7_resource_study.py"
)
parity = load_module(
    "paper1_parity", ROOT / "publication/paper1/check_access_parity.py"
)


class Paper1MeasurementToolTests(unittest.TestCase):
    def test_properties_parse_machine_output(self) -> None:
        self.assertEqual(
            resource.properties("noise\nrun_directory=C:/result\nresult_file=C:/result/result.json\n"),
            {"run_directory": "C:/result", "result_file": "C:/result/result.json"},
        )

    def test_projection_covers_authoritative_scientific_sections(self) -> None:
        document = {
            key: {"key": key}
            for key in (
                "scenario",
                "run",
                "reproducibility",
                "runtime",
                "level_b_detection",
                "level_c_assembly",
                "level_d_communication",
                "level_e_analysis",
                "validity",
            )
        }
        document["schema_version"] = "2.0.0"
        self.assertEqual(set(resource.projection(document)), set(document))

    def test_directory_metrics_count_bytes(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory(prefix="mehlissa-paper1-tools-") as directory:
            root = Path(directory)
            (root / "one.txt").write_bytes(b"123")
            (root / "two.txt").write_bytes(b"4567")
            self.assertEqual(resource.directory_metrics(root), (2, 7))

    def test_workbench_candidate_hash_is_line_ending_independent(self) -> None:
        import tempfile

        with tempfile.TemporaryDirectory(prefix="mehlissa-paper1-parity-") as directory:
            root = Path(directory)
            lf = root / "lf.json"
            crlf = root / "crlf.json"
            lf.write_bytes(b'{\n  "value": 1\n}\n')
            crlf.write_bytes(b'{\r\n  "value": 1\r\n}\r\n')
            self.assertEqual(
                parity.canonical_json_sha256(lf),
                parity.canonical_json_sha256(crlf),
            )


if __name__ == "__main__":
    unittest.main()
