# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_pulmonary_capillary_repository_audit as audit  # noqa: E402


class PulmonaryCapillaryRepositoryAuditTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = audit.load_json(audit.AUDIT)

    def test_checked_in_audit_is_valid_and_outcome_blind(self) -> None:
        document = audit.validate()
        self.assertFalse(document["decision"]["drop_in_repository_dataset_found"])
        self.assertFalse(document["decision"]["participant_files_opened"])
        self.assertFalse(document["decision"]["participant_outcomes_inspected"])
        self.assertFalse(document["decision"]["frozen_source_ranking_changed"])

    def test_parent_register_hash_is_frozen(self) -> None:
        document = copy.deepcopy(self.document)
        document["audit"]["parent_register"]["sha256"] = "0" * 64
        self.assertTrue(any("parent PCQ-1.2" in item for item in audit.errors(document)))

    def test_arizona_software_cannot_be_relabelled_as_participant_data(self) -> None:
        document = copy.deepcopy(self.document)
        target = next(item for item in document["target_studies"] if item["candidate_id"] == "PCQ-SRC-H-001")
        target["repository_status"] = "no-participant-dataset-located"
        self.assertTrue(any("Arizona" in item for item in audit.errors(document)))

    def test_alternative_cannot_be_silently_promoted(self) -> None:
        document = copy.deepcopy(self.document)
        document["alternative_sources"][0]["eligibility"] = "primary"
        self.assertTrue(audit.errors(document))

    def test_participant_file_inspection_fails_closed(self) -> None:
        document = copy.deepcopy(self.document)
        document["alternative_sources"][0]["file_inspection"] = "opened"
        self.assertTrue(audit.errors(document))

    def test_contact_queue_order_is_frozen_and_unsent(self) -> None:
        document = copy.deepcopy(self.document)
        document["decision"]["contact_queue"][0], document["decision"]["contact_queue"][1] = (
            document["decision"]["contact_queue"][1],
            document["decision"]["contact_queue"][0],
        )
        self.assertTrue(any("contact" in item for item in audit.errors(document)))


if __name__ == "__main__":
    unittest.main()
