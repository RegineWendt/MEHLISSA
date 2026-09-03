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

import check_evidence_validity_matrix as evidence  # noqa: E402


class EvidenceValidityMatrixTests(unittest.TestCase):
    def test_checked_in_matrix_is_schema_and_semantically_valid(self) -> None:
        evidence.validate(evidence.DEFAULT_MATRIX, evidence.DEFAULT_SCHEMA, ROOT)

    def test_schema_rejects_unknown_evidence_role(self) -> None:
        document = evidence.load_json(evidence.DEFAULT_MATRIX)
        document["model_families"][0]["outputs"][0]["evidence_role"] = "looks-plausible"
        schema = evidence.load_json(evidence.DEFAULT_SCHEMA)
        self.assertTrue(evidence.schema_errors(document, schema))

    def test_semantics_reject_unknown_source_and_missing_artifact(self) -> None:
        document = evidence.load_json(evidence.DEFAULT_MATRIX)
        document["model_families"][0]["claims"][0]["source_ids"].append("invented-source")
        document["model_families"][0]["artifacts"]["tests"].append(
            "tests/does-not-exist.cpp"
        )
        errors = evidence.semantic_errors(document, ROOT)
        self.assertTrue(any("invented-source" in item for item in errors))
        self.assertTrue(any("does-not-exist" in item for item in errors))

    def test_command_reports_invalid_matrix(self) -> None:
        document = evidence.load_json(evidence.DEFAULT_MATRIX)
        del document["model_families"][0]["intended_use"]
        with tempfile.TemporaryDirectory(prefix="mehlissa-evidence-negative-") as directory:
            invalid_path = Path(directory) / "invalid.json"
            invalid_path.write_text(json.dumps(document), encoding="utf-8")
            with self.assertRaises(evidence.MatrixError):
                evidence.validate(invalid_path, evidence.DEFAULT_SCHEMA, ROOT)


if __name__ == "__main__":
    unittest.main()
