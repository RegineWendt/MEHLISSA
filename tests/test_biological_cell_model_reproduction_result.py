# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_biological_cell_model_reproduction_result as result  # noqa: E402


class BiologicalCellModelReproductionResultTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = result.load_json(result.RESULT)

    def test_checked_in_result_and_archive_are_valid(self) -> None:
        document = result.validate()
        self.assertEqual(document["decision"]["source_artifact_reproduction"], "PASS")
        self.assertEqual(document["decision"]["publication_curve_alignment"], "BLOCKED")
        self.assertEqual(document["decision"]["biological_qualification"], "NOT_ESTABLISHED")

    def test_protocol_lineage_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["protocol_lineage"]["prospective_amendment"]["sha256"] = "0" * 64
        self.assertTrue(any("amendment" in item for item in result.errors(document)))

    def test_archive_manifest_hash_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["archive"]["checksum_manifest"]["sha256"] = "0" * 64
        self.assertTrue(any("checksum-manifest" in item for item in result.errors(document)))

    def test_publication_alignment_cannot_be_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["gates"][-1]["status"] = "PASS"
        self.assertTrue(result.errors(document))

    def test_m5_evidence_cannot_be_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["decision"]["m5_evidence_status"] = "externally_validated"
        self.assertTrue(result.errors(document))

    def test_replay_metric_cannot_be_rewritten(self) -> None:
        document = copy.deepcopy(self.document)
        document["headline_metrics"]["maximum_replay_fraction_of_limit"] = 0.0
        self.assertTrue(any("headline metric" in item for item in result.errors(document)))

    def test_only_final_attempt_is_authoritative(self) -> None:
        document = copy.deepcopy(self.document)
        document["attempt_history"][3]["authoritative"] = True
        self.assertTrue(any("attempt history" in item for item in result.errors(document)))


if __name__ == "__main__":
    unittest.main()
