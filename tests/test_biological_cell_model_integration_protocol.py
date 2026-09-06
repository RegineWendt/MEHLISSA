# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_biological_cell_model_integration_protocol as checker  # noqa: E402


class BiologicalCellModelIntegrationProtocolTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = checker.load_json(checker.DOCUMENT)

    def test_checked_in_protocol_passes(self) -> None:
        checker.validate(self.document)

    def test_parent_or_implementation_change_fails_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["parent_result"]["sha256"] = "0" * 64
        self.assertIn("parent result identity", " ".join(checker.errors(changed)))
        changed = copy.deepcopy(self.document)
        changed["implementation"]["source_equations"]["sha256"] = "f" * 64
        self.assertIn("source_equations", " ".join(checker.errors(changed)))

    def test_units_population_and_claims_fail_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["typed_mapping"]["state_unit"] = "nanomolar"
        self.assertIn("unit", " ".join(checker.errors(changed)))
        changed = copy.deepcopy(self.document)
        changed["population_and_uncertainty"]["precondition_met"] = True
        self.assertIn("population", " ".join(checker.errors(changed)))
        changed = copy.deepcopy(self.document)
        changed["review_and_claim_policy"]["forbidden_claims"] = []
        self.assertIn("claim safeguard", " ".join(checker.errors(changed)))

    def test_structural_companions_cannot_become_validation_runs(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["structural_sensitivity"]["trajectory_execution_in_this_protocol"] = True
        self.assertIn("structural-companion scope", " ".join(checker.errors(changed)))


if __name__ == "__main__":
    unittest.main()
