# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_pulmonary_capillary_preoutcome_amendment as amendment  # noqa: E402


class PulmonaryCapillaryPreoutcomeAmendmentTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = amendment.load(amendment.AMENDMENT)

    def rule(self, endpoint_id: str) -> dict:
        return next(
            item for item in self.document["endpoint_rules"]
            if item["endpoint_id"] == endpoint_id
        )

    def model(self, model_id: str) -> dict:
        return next(
            item for item in self.document["observation_models"]
            if item["id"] == model_id
        )

    def test_checked_in_amendment_is_valid(self) -> None:
        amendment.validate()

    def test_changed_parent_hash_is_rejected(self) -> None:
        self.document["amendment"]["parent_protocol"]["sha256"] = "0" * 64
        self.assertTrue(any("hash mismatch" in item for item in amendment.errors(self.document)))

    def test_public_outcome_exposure_cannot_be_hidden(self) -> None:
        self.document["amendment"]["public_outcome_exposure"] = "No outcomes were visible."
        self.assertTrue(any("not tuned" in item for item in amendment.errors(self.document)))

    def test_capillary_volume_margin_is_frozen(self) -> None:
        self.rule("PCQ-C1")["numeric_rule"]["geometric_mean_ratio_upper"] = 1.5
        self.assertTrue(any("PCQ-C1" in item for item in amendment.errors(self.document)))

    def test_whole_pulmonary_transit_cannot_be_silently_activated(self) -> None:
        self.rule("PCQ-C2")["activation_status"] = "locked-awaiting-data"
        self.model("PCQ-OM-C2")["status"] = "locked-awaiting-data"
        found = amendment.errors(self.document)
        self.assertTrue(any("PCQ-C2" in item for item in found))

    def test_small_arizona_cohort_cannot_become_full_track_decision(self) -> None:
        analysis_set = next(
            item for item in self.document["analysis_sets"]
            if item["track"] == "PCQ-H"
        )
        analysis_set["minimum_complete_participants_for_track_decision"] = 5
        self.assertTrue(any("PCQ-H" in item for item in amendment.errors(self.document)))

    def test_command_rejects_changed_amendment(self) -> None:
        document = copy.deepcopy(self.document)
        document["endpoint_rules"][0]["numeric_rule"]["stage_absolute_error_limit_mmHg"] = 8.0
        with tempfile.TemporaryDirectory(prefix="mehlissa-pcq-amendment-negative-") as directory:
            invalid = Path(directory) / "invalid.json"
            invalid.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(amendment.PreoutcomeAmendmentError):
                amendment.validate(invalid)


if __name__ == "__main__":
    unittest.main()
