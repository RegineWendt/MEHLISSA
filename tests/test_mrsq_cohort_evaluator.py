# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

import unittest
from scripts.mrsq_cohort_evaluator import ParticipantMetric, cohort_result, participant_metric


class MrsqEvaluatorTests(unittest.TestCase):
    def test_identical_arbitrary_curves_pass_at_sixty(self):
        metric = participant_metric([1, 2, 3], [1, 2, 3], [10, 20, 30])
        result = cohort_result([metric] * 60, nrmse_limit=.25, ratio_lower=.75, ratio_upper=1.25, require_peak=True, replicates=200)
        self.assertEqual(result.status, "PASS")
        self.assertEqual(result.median_nrmse_upper_90_ci, 0.0)

    def test_failure_is_retained(self):
        metric = participant_metric([4, 4], [1, 1], [1, 1])
        result = cohort_result([metric] * 60, nrmse_limit=.25, ratio_lower=.75, ratio_upper=1.25, replicates=200)
        self.assertEqual(result.status, "FAIL")

    def test_cohort_size_states_are_distinct(self):
        metric = ParticipantMetric(.1, 1.0, 0.0)
        self.assertEqual(cohort_result([metric] * 19, nrmse_limit=.25, ratio_lower=.75, ratio_upper=1.25, replicates=100).status, "BLOCKED")
        self.assertEqual(cohort_result([metric] * 20, nrmse_limit=.25, ratio_lower=.75, ratio_upper=1.25, replicates=100).status, "PARTIAL")

    def test_invalid_curves_fail_closed(self):
        with self.assertRaises(ValueError): participant_metric([], [], [])
        with self.assertRaises(ValueError): participant_metric([1], [0], [1])
        with self.assertRaises(ValueError): participant_metric([-1], [1], [1])

    def test_aortic_peak_gate_is_retained(self):
        metric = ParticipantMetric(.1, 1.0, 1.1)
        result = cohort_result([metric] * 60, nrmse_limit=.25, ratio_lower=.75, ratio_upper=1.25, require_peak=True, replicates=100)
        self.assertEqual(result.status, "FAIL")


if __name__ == "__main__": unittest.main()
