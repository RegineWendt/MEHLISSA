# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_medical_reference_scenario_candidates as checker  # noqa: E402


class MedicalReferenceScenarioCandidateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = checker.load_json(checker.REGISTER)

    def test_checked_in_selection_passes_without_data_or_validation_claim(self) -> None:
        document = checker.validate()
        self.assertEqual(document["decision"]["selected_candidate_id"], "MRSQ-SCN-001")
        self.assertFalse(document["screen"]["raw_participant_data_accessed"])
        self.assertFalse(document["screen"]["candidate_validation_outcomes_inspected"])
        self.assertIn("does not implement", document["decision"]["allowed_current_claim"])

    def test_selection_score_and_rank_cannot_drift(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["total_score"] -= 1
        self.assertTrue(any("total score" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["candidates"][1]["total_score"] = changed["candidates"][0]["total_score"]
        changed["candidates"][1]["scores"]["access-readiness"] += 1
        changed["candidates"][1]["scores"]["reuse-rights"] += 1
        changed["candidates"][1]["scores"]["bounded-extension"] += 1
        self.assertTrue(any("unique highest" in item for item in checker.errors(changed)))

    def test_participant_data_cannot_be_opened_by_selection_record(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["screen"]["raw_participant_data_accessed"] = True
        self.assertTrue(checker.errors(changed))

        changed = copy.deepcopy(self.document)
        changed["decision"]["access_decision"] = "public-direct"
        self.assertTrue(checker.errors(changed))

    def test_hf_data_rights_and_repository_code_rights_stay_separate(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["sources"][1]["rights"] = "NOASSERTION"
        self.assertTrue(any("Hugging Face" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["sources"][3]["rights"] = "CC-BY-4.0"
        self.assertTrue(any("NOASSERTION" in item for item in checker.errors(changed)))

    def test_source_identity_and_version_are_frozen(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["sources"][0]["persistent_locator"] = "doi:10.0/changed"
        self.assertTrue(any("MRSQ-SRC-001" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["candidates"][0]["sources"][2]["version"] = "latest"
        self.assertTrue(any("MRSQ-SRC-003" in item for item in checker.errors(changed)))

    def test_alternatives_cannot_be_promoted_past_their_missing_evidence(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["candidates"][1]["eligibility"] = "eligible-after-protocol-and-ingress-freeze"
        self.assertTrue(any("DCE-MRI" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["candidates"][3]["eligibility"] = "endpoint-only"
        self.assertTrue(any("HeartCycle" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["candidates"][4]["eligibility"] = "eligible-after-protocol-and-ingress-freeze"
        self.assertTrue(any("M7" in item for item in checker.errors(changed)))

    def test_programme_cannot_skip_prospective_protocol(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["programme"]["increments"][1]["status"] = "complete"
        self.assertTrue(any("statuses" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["decision"]["not_authorized"] = ["anything is allowed"]
        self.assertTrue(checker.errors(changed))


if __name__ == "__main__":
    unittest.main()
