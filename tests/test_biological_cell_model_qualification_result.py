# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_biological_cell_model_qualification_result as result  # noqa: E402


class BiologicalCellModelQualificationResultTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = result.load_json(result.RESULT)

    def test_checked_in_result_and_archives_are_valid(self) -> None:
        document = result.validate()
        self.assertEqual(document["decision"]["cross_engine_reproduction"], "PASS")
        self.assertEqual(document["decision"]["population_ensemble"], "BLOCKED")
        self.assertEqual(document["decision"]["biological_qualification"], "NOT_ESTABLISHED")

    def test_protocol_identity_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["protocol"]["sha256"] = "0" * 64
        self.assertTrue(any("protocol" in item for item in result.errors(document)))

    def test_parent_copasi_result_identity_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["parent_result"]["sha256"] = "0" * 64
        self.assertTrue(any("parent" in item for item in result.errors(document)))

    def test_archive_manifest_identity_cannot_drift(self) -> None:
        document = copy.deepcopy(self.document)
        document["archive"]["checksum_manifest"]["sha256"] = "0" * 64
        self.assertTrue(any("manifest" in item for item in result.errors(document)))

    def test_raw_trajectory_metric_cannot_be_rewritten(self) -> None:
        document = copy.deepcopy(self.document)
        document["headline_metrics"]["maximum_cross_engine_fraction_of_limit"] = 0.0
        self.assertTrue(any("headline metric" in item for item in result.errors(document)))

    def test_source_case_identity_cannot_be_swapped(self) -> None:
        document = copy.deepcopy(self.document)
        document["execution"]["source_accessions"].reverse()
        self.assertTrue(result.errors(document))

    def test_stimulus_and_unit_contract_cannot_be_relabelled(self) -> None:
        document = copy.deepcopy(self.document)
        document["execution"]["state_unit"] = "nanomolar"
        self.assertTrue(result.errors(document))

    def test_parameter_refitting_cannot_be_enabled(self) -> None:
        document = copy.deepcopy(self.document)
        document["execution"]["parameter_refitting"] = True
        self.assertTrue(result.errors(document))

    def test_output_grid_cannot_be_truncated(self) -> None:
        document = copy.deepcopy(self.document)
        document["execution"]["points_per_run"] = 960
        self.assertTrue(result.errors(document))

    def test_convergence_metric_cannot_be_rewritten(self) -> None:
        document = copy.deepcopy(self.document)
        document["headline_metrics"]["maximum_mehlissa_convergence_fraction_of_limit"] = 0.0
        self.assertTrue(any("headline metric" in item for item in result.errors(document)))

    def test_invariant_metric_cannot_be_rewritten(self) -> None:
        document = copy.deepcopy(self.document)
        document["headline_metrics"]["maximum_invariant_residual"] = 0.0
        self.assertTrue(any("headline metric" in item for item in result.errors(document)))

    def test_structural_companions_cannot_be_promoted_to_independent_validation(self) -> None:
        document = copy.deepcopy(self.document)
        document["structural_scope"]["independent_validation"] = True
        self.assertTrue(result.errors(document))

    def test_population_gate_cannot_be_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["gates"][8]["status"] = "PASS"
        self.assertTrue(result.errors(document))

    def test_biological_qualification_cannot_be_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["decision"]["biological_qualification"] = "PASS"
        self.assertTrue(result.errors(document))

    def test_published_and_synthetic_evidence_classes_cannot_be_conflated(self) -> None:
        document = copy.deepcopy(self.document)
        document["decision"]["synthetic_m5_fixture_evidence"] = (
            "computationally-qualified-published-average-cell-mechanism"
        )
        self.assertTrue(result.errors(document))

    def test_only_final_attempt_is_authoritative(self) -> None:
        document = copy.deepcopy(self.document)
        document["attempt_history"][1]["authoritative"] = True
        self.assertTrue(any("attempt" in item for item in result.errors(document)))


if __name__ == "__main__":
    unittest.main()
