# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/check_dynamic_capillary_tissue_cell_protocol.py"
SPEC = importlib.util.spec_from_file_location("dccq_protocol", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class DynamicProtocolTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = MODULE.load_json(MODULE.PROTOCOL)

    def assert_rejected(self, document: dict, fragment: str) -> None:
        found = MODULE.errors(document)
        self.assertTrue(any(fragment in item for item in found), found)

    def test_checked_in_protocol_passes(self) -> None:
        self.assertEqual([], MODULE.errors(self.document))

    def test_changed_ligand_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["target"]["ligand_id"] = "oxygen"
        self.assert_rejected(changed, "VEGF-A165a identity")

    def test_incorrect_si_conversion_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["si_mapping"]["source_rab11_surface_m2_per_cell"] = 3.25e-10
        self.assert_rejected(changed, "source_rab11_surface")

    def test_missing_owner_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["state_contract"]["owners"].remove("internalized")
        self.assert_rejected(changed, "seven-owner ledger")

    def test_same_step_feedback_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["synchronization"]["feedback_rule"] = "Apply in interval n."
        self.assert_rejected(changed, "explicitly delayed")

    def test_refitting_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["evaluation"]["validation_refitting"] = True
        self.assert_rejected(changed, "refitting")

    def test_asset_mutation_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["frozen_implementation"][0]["sha256"] = "0" * 64
        self.assert_rejected(changed, "asset hash changed")


if __name__ == "__main__":
    unittest.main()
