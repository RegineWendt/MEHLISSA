# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_rq4_campaign as campaign


class Rq4CampaignTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.plan_path = campaign.REPOSITORY_ROOT / "examples/benchmarks/rq4-primary-campaign.json"
        cls.plan = campaign.load_json(cls.plan_path)

    def test_frozen_plan_and_condition_matrix_are_consistent(self) -> None:
        campaign.validate_plan(self.plan, executing=False, pilot=False)
        campaign.validate_frozen_pilot(self.plan, self.plan_path)
        conditions = campaign.mandatory_conditions(self.plan)
        self.assertEqual(len(conditions), 11)
        self.assertEqual(sum(item.policy_id == "O3" for item in conditions), 1)
        self.assertEqual([item.population for item in conditions if item.policy_id == "O3"], [1000])
        self.assertEqual([item.key for item in conditions if item.anchor], ["anchor-n6359-o0"])

    def test_every_measured_block_contains_each_condition_once(self) -> None:
        conditions = campaign.mandatory_conditions(self.plan)
        schedule = campaign.build_schedule(self.plan, conditions, "mandatory")
        self.assertEqual(len(schedule), 88)
        self.assertEqual(sum(item["kind"] == "warmup" for item in schedule), 11)
        expected = sorted(item.key for item in conditions)
        observed_orders = []
        for block in range(1, 8):
            entries = [item["condition"] for item in schedule if item["block"] == block]
            self.assertEqual(sorted(entries), expected)
            observed_orders.append(entries)
        self.assertGreater(len({tuple(order) for order in observed_orders}), 1)

    def test_generated_manifest_omits_campaign_only_controls(self) -> None:
        manifest = campaign.create_manifest(
            self.plan, self.plan_path, campaign.Condition(10_000, "O2"), "b01"
        )
        self.assertEqual(manifest["injection"]["particle_count"], 10_000)
        self.assertEqual(manifest["observation"]["policy_id"], "O2")
        self.assertNotIn("required_truncation", manifest["observation"])
        self.assertTrue(Path(manifest["model"]["path"]).is_absolute())

    def test_seven_value_summary_preserves_raw_values(self) -> None:
        values = [7, 1, 6, 2, 5, 3, 4]
        summary = campaign.quartile_summary(values)
        self.assertEqual(summary["median"], 4)
        self.assertEqual(summary["minimum"], 1)
        self.assertEqual(summary["maximum"], 7)
        self.assertEqual(summary["all"], values)

    def test_dry_run_writes_a_complete_planned_ledger(self) -> None:
        with tempfile.TemporaryDirectory(prefix="mehlissa-rq4-test-") as directory:
            command = [
                sys.executable,
                str(campaign.REPOSITORY_ROOT / "benchmarks/run_rq4_campaign.py"),
                "--plan",
                str(self.plan_path),
                "--output-directory",
                directory,
                "--dry-run",
            ]
            completed = subprocess.run(command, capture_output=True, text=True, check=False)
            self.assertEqual(completed.returncode, 0, completed.stderr)
            with (Path(directory) / "campaign-report.json").open(encoding="utf-8") as source:
                report = json.load(source)
            self.assertEqual(report["status"], "planned")
            self.assertEqual(len(report["schedule"]), 88)
            self.assertFalse(report["suitable_for_analysis"])


if __name__ == "__main__":
    unittest.main()
