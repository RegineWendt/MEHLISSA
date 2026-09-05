# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import math
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_pulmonary_capillary_uncertainty as uncertainty  # noqa: E402


class PulmonaryCapillaryUncertaintyTests(unittest.TestCase):
    def setUp(self) -> None:
        self.plan = uncertainty.load_json(uncertainty.PLAN)

    def test_plan_covers_six_classes_nine_endpoints_and_frozen_assets(self) -> None:
        document = uncertainty.validate_plan()
        summary = uncertainty.summarize(document)
        self.assertEqual(6, summary.uncertainty_class_count)
        self.assertEqual(9, summary.endpoint_count)
        self.assertEqual(7, summary.structural_model_count)
        self.assertEqual(63, summary.structural_grid_case_count)
        self.assertFalse(summary.measured_outcomes_used)
        self.assertFalse(summary.qualification_decisions_emitted)

    def test_structural_ensemble_retains_v4_v7_equivalence_and_real_spread(self) -> None:
        predictions = uncertainty.structural_predictions(self.plan)
        by_key = {(item.model_id, item.age_years, item.flow_L_min): item for item in predictions}
        v4 = "lung.pulmonary-0d.healthy-adult-rest-exercise-age-invasive.v4"
        v7 = "lung.pulmonary-0d.healthy-adult-lobar-parallel.v7"
        for age in self.plan["analysis_grid"]["age_years"]:
            for flow in self.plan["analysis_grid"]["flow_L_min"]:
                self.assertEqual(by_key[(v4, age, flow)], by_key[(v7, age, flow)].__class__(
                    model_id=v4,
                    age_years=age,
                    flow_L_min=flow,
                    mPAP_mmHg=by_key[(v7, age, flow)].mPAP_mmHg,
                    PVR_WU=by_key[(v7, age, flow)].PVR_WU,
                    compliance_mL_mmHg=by_key[(v7, age, flow)].compliance_mL_mmHg,
                    RC_time_s=by_key[(v7, age, flow)].RC_time_s,
                ))
        high_flow = [item.mPAP_mmHg for item in predictions if item.age_years == 30.0 and item.flow_L_min == 14.0]
        self.assertGreater(max(high_flow) - min(high_flow), 0.5)

    def test_observational_propagation_retains_floors_and_covariance_envelopes(self) -> None:
        below_floor = uncertainty.pvr_standard_uncertainty_envelope(15.0, 8.0, 5.5, 0.1, 0.1, 0.01)
        at_floor = uncertainty.pvr_standard_uncertainty_envelope(15.0, 8.0, 5.5, 2.0, 3.0, 0.10)
        self.assertEqual(below_floor, at_floor)
        self.assertGreater(at_floor[1], at_floor[0])
        self.assertTrue(math.isclose(uncertainty.log_standard_uncertainty_from_cv(0.08), 0.07987244183095335, rel_tol=1.0e-12))
        ratio_lower, ratio_upper = uncertainty.ratio_relative_uncertainty_envelope(0.08, 0.10)
        self.assertAlmostEqual(0.02, ratio_lower)
        self.assertAlmostEqual(0.18, ratio_upper)

    def test_local_sensitivities_are_signed_and_converged(self) -> None:
        records = uncertainty.sensitivity_records(self.plan)
        self.assertEqual(7, len(records))
        self.assertTrue(all(item.converged for item in records))
        directions = {(item.parameter, item.output): item.signed_direction for item in records}
        self.assertEqual("positive", directions[("functional_capillary_volume", "capillary_residence_s")])
        self.assertEqual("negative", directions[("cardiac_output", "capillary_residence_s")])
        self.assertEqual("negative", directions[("flow_resistance_exponent", "PVR_WU")])

    def test_identifiability_ranks_expose_confounding_and_dynamic_information(self) -> None:
        matrices = uncertainty.identifiability_matrices(self.plan["analysis_grid"]["flow_L_min"])
        ranks = {identifier: uncertainty.matrix_rank(matrix) for identifier, matrix in matrices.items()}
        self.assertEqual(1, ranks["ID-H-SINGLE"])
        self.assertEqual(2, ranks["ID-H-MULTIPOINT"])
        self.assertEqual(2, ranks["ID-H-EQUILIBRIUM"])
        self.assertEqual(4, ranks["ID-H-DYNAMIC"])
        self.assertEqual(4, ranks["ID-R-COMPOSITION"])
        self.assertEqual(1, ranks["ID-C-GEOMETRY"])
        self.assertEqual(1, ranks["ID-C2-DELAYS"])
        self.assertEqual(2, ranks["ID-J-CLOSURE"])

    def test_global_variance_and_whole_transit_remain_explicitly_blocked(self) -> None:
        boundary = self.plan["decision_boundary"]
        self.assertIn("blocked-until", boundary["global_variance_analysis"])
        self.assertEqual("blocked-observation-model", boundary["transit_endpoint"])
        c2 = next(item for item in self.plan["observational_models"] if item["endpoint_id"] == "PCQ-C2")
        self.assertEqual("blocked-observation-model", c2["status"])

    def test_parent_hash_model_hash_and_declared_rank_fail_closed(self) -> None:
        document = copy.deepcopy(self.plan)
        document["plan"]["parent_ingress"]["sha256"] = "0" * 64
        self.assertTrue(any("parent PCQ-1.4" in item for item in uncertainty.plan_errors(document)))
        document = copy.deepcopy(self.plan)
        document["model_assets"][0]["sha256"] = "0" * 64
        self.assertTrue(any("model asset" in item for item in uncertainty.plan_errors(document)))
        document = copy.deepcopy(self.plan)
        document["identifiability_analyses"][0]["expected_rank"] = 2
        self.assertTrue(any("declared rank" in item for item in uncertainty.plan_errors(document)))

    def test_design_does_not_enable_refit_or_outcome_claims(self) -> None:
        boundary = self.plan["decision_boundary"]
        self.assertEqual("forbidden", boundary["validation_refit"])
        self.assertEqual("software-test-only", boundary["synthetic_evidence"])
        self.assertFalse(boundary["raw_observations_emitted"])
        self.assertFalse(boundary["qualification_decisions_emitted"])
        self.assertIn("No candidate participant-level outcome", self.plan["plan"]["outcome_access"])


if __name__ == "__main__":
    unittest.main()
