# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import sys
import unittest

from mehlissa import MehlissaClient, MehlissaCommandError, load_campaign_result, load_result


PARSER = argparse.ArgumentParser()
PARSER.add_argument("--executable", required=True)
PARSER.add_argument("--root", required=True)
PARSER.add_argument("--output", required=True)
ARGS = PARSER.parse_args()


class PythonApiTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(ARGS.root).resolve()
        cls.output = Path(ARGS.output).resolve()
        shutil.rmtree(cls.output, ignore_errors=True)
        cls.output.mkdir(parents=True)
        cls.client = MehlissaClient(ARGS.executable, cls.root)

    def test_complete_process_and_analysis_workflow(self) -> None:
        models = self.client.list_models(layer="organ")
        self.assertIn("organ.pulmonary-zero-dimensional", models)

        scenario_file = self.root / "examples/scenarios/fp9-lung-level-a-v1.json"
        self.assertIn("Scenario is valid", self.client.validate_scenario(scenario_file))
        execution = self.client.run_scenario(scenario_file, self.output / "scenario")
        self.assertTrue(execution.result.is_file())
        self.assertTrue(execution.provenance.is_file())

        result = load_result(execution.result)
        self.assertEqual(result.summary["scenario_id"], "fp9-lung-level-a-v1")
        self.assertTrue(result.summary["detected"])
        self.assertTrue(result.summary["assembled"])
        self.assertFalse(result.summary["clinical_validation_claim"])
        self.assertEqual(len(result.runtime_stages), 10)
        self.assertEqual(len(result.analysis_cases), 4)

        report = self.client.report_result(execution.result, self.output / "report")
        self.assertTrue(report.html.is_file())
        self.assertTrue(report.result.is_file())
        with self.assertRaises(MehlissaCommandError):
            self.client.report_result(execution.result, self.output / "report")

        campaign_file = self.root / "examples/campaigns/fp9-collector-count-v1.json"
        self.assertIn("6 derived runs", self.client.validate_campaign(campaign_file))
        campaign_execution = self.client.run_campaign(
            campaign_file, self.output / "campaign"
        )
        self.assertEqual(campaign_execution.derived_runs, 6)
        self.assertTrue(campaign_execution.csv.is_file())

        campaign = load_campaign_result(campaign_execution.result)
        self.assertEqual(len(campaign.runs), 6)
        self.assertEqual(set(campaign.groups()), {
            "replicates", "collector-count-sweep", "collector-count-pair"
        })
        pairs = campaign.paired_differences("sensitivity")
        self.assertEqual(len(pairs), 1)
        self.assertEqual(pairs[0]["seed"], 20260930)
        self.assertEqual(pairs[0]["difference"], 0.0)
        self.assertEqual(len(campaign.metric_series("specificity")), 6)
        with self.assertRaises(ValueError):
            campaign.metric_series("clinical_accuracy")

    def test_version_guards_reject_unrelated_json(self) -> None:
        with self.assertRaises(ValueError):
            load_result(self.root / "examples/campaigns/fp9-collector-count-v1.json")
        with self.assertRaises(ValueError):
            load_campaign_result(self.root / "examples/scenarios/fp9-lung-level-a-v1.json")


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]], verbosity=2)
