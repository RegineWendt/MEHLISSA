# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_biological_cell_model_candidates as candidates  # noqa: E402


class BiologicalCellModelCandidateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = candidates.load_json(candidates.REGISTER)

    def test_checked_in_register_is_valid_and_pre_import(self) -> None:
        document = candidates.validate()
        self.assertEqual(document["decision"]["selected_candidate_id"], "BCQ-SRC-001")
        self.assertFalse(document["screen"]["clinical_use"])
        self.assertIn("No external model", document["screen"]["selection_boundary"])

    def test_selected_artifact_hash_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["candidates"][0]["artifacts"][0]["sha256"] = "0" * 64
        self.assertTrue(any("sha256" in item for item in candidates.errors(document)))

    def test_score_total_must_equal_dimension_sum(self) -> None:
        document = copy.deepcopy(self.document)
        document["candidates"][0]["total_score"] -= 1
        self.assertTrue(any("total score" in item for item in candidates.errors(document)))

    def test_larger_same_paper_variant_cannot_be_called_independent(self) -> None:
        document = copy.deepcopy(self.document)
        document["candidates"][0]["structural_companions"] = []
        self.assertTrue(candidates.errors(document))

    def test_model_license_cannot_relicense_article_or_data(self) -> None:
        document = copy.deepcopy(self.document)
        document["candidates"][0]["publication"]["article_rights"] = "CC0-1.0"
        self.assertTrue(candidates.errors(document))

    def test_unreleased_endothelial_submission_cannot_be_silently_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["candidates"][2]["decision"] = "selected-model-family"
        self.assertTrue(candidates.errors(document))


if __name__ == "__main__":
    unittest.main()
