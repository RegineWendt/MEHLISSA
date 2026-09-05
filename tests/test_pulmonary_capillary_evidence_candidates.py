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

import check_pulmonary_capillary_evidence_candidates as register  # noqa: E402


class PulmonaryCapillaryEvidenceCandidateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.document = register.load(register.REGISTER)

    def candidate(self, candidate_id: str) -> dict:
        return next(item for item in self.document["candidates"] if item["id"] == candidate_id)

    def test_checked_in_register_is_valid(self) -> None:
        register.validate()

    def test_duplicate_candidate_identifier_is_rejected(self) -> None:
        document = copy.deepcopy(self.document)
        document["candidates"][1]["id"] = document["candidates"][0]["id"]
        self.assertTrue(any("candidate identifiers" in item for item in register.errors(document)))

    def test_pvdomics_cannot_be_promoted_to_primary(self) -> None:
        self.candidate("PCQ-SRC-H-004")["decision"] = "priority-request"
        self.assertTrue(any("PVDOMICS" in item for item in register.errors(self.document)))

    def test_public_outcome_exposure_cannot_be_hidden(self) -> None:
        self.candidate("PCQ-SRC-CJ-001")["outcome_exposure"]["status"] = "metadata-and-methods-only"
        self.assertTrue(any("D'Souza" in item for item in register.errors(self.document)))

    def test_circular_transit_volume_limitation_is_required(self) -> None:
        candidate = self.candidate("PCQ-SRC-C-003")
        candidate["jointness"] = "All measurements are independent."
        candidate["gaps"] = ["Participant rights pending."]
        self.assertTrue(any("circular-closure" in item for item in register.errors(self.document)))

    def test_command_validation_rejects_unsent_request_without_boundary(self) -> None:
        document = copy.deepcopy(self.document)
        document["next_actions"][0]["authorization"] = "already sent"
        with tempfile.TemporaryDirectory(prefix="mehlissa-pcq-source-negative-") as directory:
            invalid = Path(directory) / "invalid.json"
            invalid.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(register.EvidenceCandidateRegisterError):
                register.validate(invalid)


if __name__ == "__main__":
    unittest.main()
