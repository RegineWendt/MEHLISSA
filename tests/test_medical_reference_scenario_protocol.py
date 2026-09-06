# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_medical_reference_scenario_protocol as checker  # noqa: E402


class MedicalReferenceScenarioProtocolTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = checker.load_json(checker.PROTOCOL)

    def test_checked_in_protocol_is_prospective_and_passes(self) -> None:
        document = checker.validate()
        self.assertFalse(document["governance"]["participant_files_opened"])
        self.assertFalse(document["governance"]["validation_outcomes_inspected"])
        self.assertFalse(document["governance"]["ingress_authorized"])
        self.assertEqual(document["next_increment"], "MRSQ-1.3 rights-aware selective FDG-PET data ingress")

    def test_parent_selection_and_remote_revision_are_immutable(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["parent_selection"]["sha256"] = "0" * 64
        self.assertTrue(any("parent selection hash" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["source_freeze"]["data_repository"]["revision"] = "1" * 40
        self.assertTrue(any("Hugging Face revision" in item for item in checker.errors(changed)))

    def test_minimum_data_cannot_expand_or_open_early(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["source_freeze"]["data_repository"]["minimum_assets"].pop()
        self.assertTrue(checker.errors(changed))

        changed = copy.deepcopy(self.document)
        changed["source_freeze"]["data_repository"]["minimum_assets"][2]["ingress_state"] = "metadata-only-download-permitted"
        self.assertTrue(any("ingress state" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["governance"]["ingress_authorized"] = True
        self.assertTrue(checker.errors(changed))

    def test_unlicensed_code_and_full_images_stay_out_of_scope(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["source_freeze"]["processing_repository"]["rights"] = "CC-BY-4.0"
        self.assertTrue(any("processing-code" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["source_freeze"]["full_image_fallback"]["access"] = "download now"
        self.assertTrue(any("full-image" in item for item in checker.errors(changed)))

    def test_conditional_input_cannot_become_primary(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["analysis_sets"][1]["qualification_role"] = "primary"
        self.assertTrue(any("analysis roles" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["endpoint_rules"][1]["analysis_set"] = "MRSQ-AS-CONDITIONAL"
        self.assertTrue(checker.errors(changed))

    def test_region_components_cannot_be_selected_after_outcomes(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["regions"][0]["required_source_components"].pop()
        self.assertTrue(any("region composition" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["regions"][2]["required_source_components"] = ["kidney_left"]
        self.assertTrue(any("region composition" in item for item in checker.errors(changed)))

    def test_numeric_gates_and_minimum_cohort_cannot_drift(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["endpoint_rules"][0]["limits"]["cohort_upper_90_ci_median_duration_weighted_nrmse"] = 0.5
        self.assertTrue(any("numeric gate" in item for item in checker.errors(changed)))

        changed = copy.deepcopy(self.document)
        changed["endpoint_rules"][4]["minimum_complete_participants"] = 20
        self.assertTrue(checker.errors(changed))

    def test_outcome_access_and_post_outcome_rescue_fail_closed(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["governance"]["validation_outcomes_inspected"] = True
        self.assertTrue(checker.errors(changed))

        changed = copy.deepcopy(self.document)
        changed["failure_policy"]["amendment"] = "Change limits until the result passes."
        self.assertTrue(any("amendment policy" in item for item in checker.errors(changed)))

    def test_clinical_or_primary_pass_claim_is_rejected(self) -> None:
        changed = copy.deepcopy(self.document)
        changed["forbidden_claims"] = ["nothing"]
        self.assertTrue(checker.errors(changed))

        changed = copy.deepcopy(self.document)
        changed["allowed_claim"] = "All endpoints pass and clinical validity is established."
        self.assertTrue(any("allowed claim" in item for item in checker.errors(changed)))


if __name__ == "__main__":
    unittest.main()
