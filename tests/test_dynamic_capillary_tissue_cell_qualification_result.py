# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/check_dynamic_capillary_tissue_cell_qualification_result.py"
SPEC = importlib.util.spec_from_file_location("dccq_result", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class DynamicQualificationResultTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = MODULE.load_json(MODULE.RESULT)

    def assert_rejected(self, document: dict, fragment: str) -> None:
        found = MODULE.errors(document)
        self.assertTrue(any(fragment in item for item in found), found)

    def test_checked_in_result_passes(self) -> None:
        self.assertEqual([], MODULE.errors(self.document))

    def test_gate_overclaim_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["gates"][6]["status"] = "PASS"
        self.assert_rejected(changed, "overclaims external evidence")

    def test_clinical_status_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["result"]["clinical_use"] = True
        self.assert_rejected(changed, "False was expected")

    def test_manifest_mutation_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["archive"]["checksum_manifest"]["sha256"] = "0" * 64
        self.assert_rejected(changed, "manifest hash changed")

    def test_protocol_mutation_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["protocol"]["sha256"] = "f" * 64
        self.assert_rejected(changed, "protocol hash changed")

    def test_biological_overclaim_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["decision"]["biological_or_clinical_validation"] = "PASS"
        self.assert_rejected(changed, "overclaimed")


if __name__ == "__main__":
    unittest.main()
