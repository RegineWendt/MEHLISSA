# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_dynamic_capillary_tissue_cell_qualification_plan as checker  # noqa: E402


class DynamicCapillaryTissueCellQualificationPlanTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = checker.load_json(checker.DOCUMENT)

    def test_checked_in_plan_passes(self) -> None:
        checker.validate(self.document)

    def test_snapshot_and_cd95_shortcuts_fail_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["baseline_components"][1]["blocking_gaps"] = ["none"]
        self.assertIn("snapshot non-consumption", " ".join(checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["baseline_components"][3]["disposition"] = (
            "retain-as-transport-verification-baseline"
        )
        self.assertIn("baseline evidence role", " ".join(checker.errors(changed)))

    def test_units_identity_and_ownership_fail_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["cross_layer_state_contract"]["unit_policy"] = "Assume every value is SI."
        self.assertIn("unresolved-unit", " ".join(checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["cross_layer_state_contract"]["ownership_ledger"][3] = "untracked_pool"
        self.assertIn("ownership ledger", " ".join(checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["cross_layer_state_contract"]["ligand_identity_policy"] = (
            "Aliases may map different substances."
        )
        self.assertIn("biochemical identity", " ".join(checker.errors(changed)))

    def test_gates_and_future_statuses_cannot_be_predeclared_passed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["planned_gates"][0]["current_state"] = "PASS"
        self.assertIn("schema", " ".join(checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["increments"][1]["status"] = "COMPLETE"
        self.assertIn("next-step status", " ".join(checker.errors(changed)))

    def test_frozen_asset_and_claim_scope_fail_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["frozen_baseline_assets"][0]["sha256"] = "0" * 64
        self.assertIn("asset hash changed", " ".join(checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["governance_and_claim_policy"]["forbidden_claims"] = [
            "unrelated claim one",
            "unrelated claim two",
            "unrelated claim three",
            "unrelated claim four",
            "unrelated claim five",
        ]
        self.assertIn("claim safeguard", " ".join(checker.errors(changed)))


if __name__ == "__main__":
    unittest.main()
