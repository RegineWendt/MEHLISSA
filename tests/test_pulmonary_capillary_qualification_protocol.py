# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_pulmonary_capillary_qualification_protocol as protocol  # noqa: E402


class PulmonaryCapillaryQualificationProtocolTests(unittest.TestCase):
    def test_checked_in_protocol_is_valid_and_assets_are_frozen(self) -> None:
        protocol.validate()

    def test_schema_rejects_a_supported_candidate_claim(self) -> None:
        document = protocol.load(protocol.PROTOCOL)
        document["candidate_claim"]["evidence_status"] = "supported"
        self.assertTrue(protocol.errors(document))

    def test_semantics_reject_a_changed_frozen_asset(self) -> None:
        document = protocol.load(protocol.PROTOCOL)
        document["frozen_assets"][0]["sha256"] = "0" * 64
        self.assertTrue(any("hash mismatch" in item for item in protocol.errors(document)))

    def test_semantics_reject_duplicate_endpoint(self) -> None:
        document = protocol.load(protocol.PROTOCOL)
        document["endpoints"][1]["id"] = document["endpoints"][0]["id"]
        self.assertTrue(any("endpoint hierarchy" in item for item in protocol.errors(document)))

    def test_command_validation_rejects_refitting(self) -> None:
        document = protocol.load(protocol.PROTOCOL)
        document["analysis"]["no_refit_on_validation"] = False
        with tempfile.TemporaryDirectory(prefix="mehlissa-pcq-negative-") as directory:
            invalid = Path(directory) / "invalid.json"
            invalid.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(protocol.QualificationProtocolError):
                protocol.validate(invalid)


if __name__ == "__main__":
    unittest.main()
