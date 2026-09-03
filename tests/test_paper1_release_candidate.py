# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

import jsonschema


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/check_paper1_release_candidate.py"
spec = importlib.util.spec_from_file_location("paper1_release", SCRIPT)
if spec is None or spec.loader is None:
    raise RuntimeError(f"Cannot load {SCRIPT}")
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class Paper1ReleaseCandidateTests(unittest.TestCase):
    def test_release_candidate_is_complete(self) -> None:
        manifest = release.validate_candidate()
        self.assertEqual(manifest["status"], "review_candidate")
        self.assertEqual(len(manifest["measurements"]), 3)

    def test_claim_semantics_reject_supported_clinical_claim(self) -> None:
        schema = release.load_json(
            ROOT / "data/schemas/paper1-claim-registry/1.0.0.schema.json"
        )
        registry = release.load_json(
            release.DEFAULT_CANDIDATE / "claim-to-artifact-registry.json"
        )
        invalid = copy.deepcopy(registry)
        clinical = next(claim for claim in invalid["claims"] if claim["category"] == "clinical")
        clinical["status"] = "supported"
        jsonschema.validate(invalid, schema)
        with self.assertRaisesRegex(ValueError, "overstates biological or clinical"):
            release.verify_claim_semantics(invalid)

    def test_release_schema_rejects_final_tag_state(self) -> None:
        schema = release.load_json(
            ROOT / "data/schemas/paper1-release-candidate/1.0.0.schema.json"
        )
        manifest = release.load_json(release.DEFAULT_CANDIDATE / "release-candidate.json")
        invalid = copy.deepcopy(manifest)
        invalid["boundaries"]["final_tag_created"] = True
        with self.assertRaises(jsonschema.ValidationError):
            jsonschema.validate(invalid, schema)

    def test_zip_rejects_path_escape(self) -> None:
        import zipfile

        with tempfile.TemporaryDirectory(prefix="mehlissa-paper1-zip-") as directory:
            archive = Path(directory) / "unsafe.zip"
            with zipfile.ZipFile(archive, "w") as output:
                output.writestr("../escape.txt", "no")
            with self.assertRaises(ValueError):
                release.verify_zip(archive, ())


if __name__ == "__main__":
    unittest.main()
