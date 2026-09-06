# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_dynamic_capillary_tissue_cell_evidence_candidates as checker  # noqa: E402


class DynamicCapillaryTissueCellEvidenceCandidateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = checker.load_json(checker.REGISTER)

    def test_checked_in_register_passes_without_implementation_claim(self) -> None:
        document = checker.validate()
        self.assertEqual(document["decision"]["selected_candidate_id"], "DCCQ-SRC-001")
        self.assertFalse(document["screen"]["clinical_use"])
        self.assertIn("does not implement or qualify", document["decision"]["allowed_current_claim"])

    def test_parent_plan_and_source_artifact_identity_cannot_drift(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["screen"]["parent_plan_sha256"] = "0" * 64
        self.assertIn("parent DCCQ-1.1 plan hash changed", checker.errors(changed))

        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["artifacts"][1]["sha256"] = "0" * 64
        self.assertTrue(any("DCCQ-ART-002" in item for item in checker.errors(changed)))

    def test_score_and_selection_must_remain_consistent(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["total_score"] -= 1
        self.assertTrue(any("total score" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["candidates"][1]["total_score"] = changed["candidates"][0]["total_score"]
        changed["candidates"][1]["scores"]["artifact-identity"] += 2
        changed["candidates"][1]["scores"]["source-disjoint-evidence"] += 1
        self.assertTrue(any("unique highest" in item for item in checker.errors(changed)))

    def test_unlicensed_repository_cannot_be_silently_reused(self) -> None:
        changed = copy.deepcopy(self.document)
        artifact = changed["candidates"][0]["artifacts"][1]
        artifact["licence"] = "CC-BY-4.0"
        artifact["reuse_decision"] = "eligible-with-attribution"
        self.assertTrue(checker.errors(changed))

    def test_huvec_calibration_family_cannot_be_called_independent(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["evidence_sources"][0]["role"] = "source-disjoint-no-refit-kinetic-challenge"
        self.assertTrue(any("same-family HUVEC" in item for item in checker.errors(changed)))

    def test_context_mismatch_and_g7_blocker_fail_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["evidence_sources"][3]["eligibility"] = "eligible-not-independent"
        self.assertTrue(any("Peach" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["decision"]["gate_implications"]["DCCQ-G7"] = "PASS"
        self.assertTrue(any("DCCQ-G7" in item for item in checker.errors(changed)))

    def test_selected_source_units_cannot_be_predeclared_si_frozen(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["unit_assessment"]["si_bridge_status"] = "blocked"
        self.assertTrue(any("already SI-frozen" in item for item in checker.errors(changed)))


if __name__ == "__main__":
    unittest.main()
