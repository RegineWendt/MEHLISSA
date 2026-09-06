# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

import copy
import importlib.util
import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("mrsq_ingress", ROOT / "scripts/check_medical_reference_scenario_data_ingress.py")
ingress = importlib.util.module_from_spec(SPEC)
assert SPEC.loader
sys.modules[SPEC.name] = ingress
SPEC.loader.exec_module(ingress)


class MrsqIngressTests(unittest.TestCase):
    def manifest(self):
        return ingress.load_json(ingress.FIXTURE_MANIFEST)

    def write_manifest(self, folder: Path, document):
        path = folder / "manifest.json"
        path.write_text(json.dumps(document), encoding="utf-8")
        return path

    def test_synthetic_fixture_passes_without_exposing_values(self):
        result = ingress.ingest(allow_synthetic=True)
        self.assertEqual((result.participant_count, result.aortic_frame_count, result.region_count), (1, 3, 9))
        self.assertFalse(result.source_values_exposed)

    def test_synthetic_requires_explicit_release(self):
        with self.assertRaisesRegex(ingress.MrsqIngressError, "allow_synthetic"):
            ingress.ingest()

    def test_policy_parent_hash_is_locked(self):
        policy = ingress.load_json(ingress.POLICY)
        policy["policy"]["parent_protocol"]["sha256"] = "0" * 64
        self.assertTrue(any("parent protocol" in item for item in ingress.policy_errors(policy)))

    def test_manifest_protocol_hash_is_locked(self):
        document = self.manifest(); document["protocol"]["sha256"] = "0" * 64
        self.assertTrue(any("protocol identity" in item for item in ingress.manifest_errors(document, ingress.load_json(ingress.POLICY))))

    def test_revision_is_locked(self):
        document = self.manifest(); document["protocol"]["data_revision"] = "0" * 40
        self.assertTrue(any("data revision" in item for item in ingress.manifest_errors(document, ingress.load_json(ingress.POLICY))))

    def test_measured_data_fails_before_csv_open(self):
        document = self.manifest()
        document["manifest"]["evidence_status"] = "measured_validation"
        document["source"].update(provider="Hugging Face", repository_id="DEPICT-RH/Multimodal-HC", raw_redistribution="prohibited")
        document["governance"]["local_determination"] = "pending"
        with tempfile.TemporaryDirectory() as directory:
            path = self.write_manifest(Path(directory), document)
            with patch.object(ingress, "_read_rows", side_effect=AssertionError("CSV opened")):
                with self.assertRaisesRegex(ingress.MrsqIngressError, "measured ingress is blocked"):
                    ingress.ingest(path)

    def test_duplicate_role_is_rejected(self):
        document = self.manifest(); document["content"]["assets"][1]["role"] = "administration"
        found = ingress.manifest_errors(document, ingress.load_json(ingress.POLICY))
        self.assertTrue(any("four frozen roles" in item for item in found))

    def test_region_mapping_is_locked(self):
        document = self.manifest(); document["semantics"]["region_components"]["MRSQ-ROI-LUNG"].pop()
        found = ingress.manifest_errors(document, ingress.load_json(ingress.POLICY))
        self.assertTrue(any("region mapping" in item for item in found))

    def test_hash_mismatch_fails(self):
        document = self.manifest(); document["content"]["assets"][0]["sha256"] = "0" * 64
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            for asset in document["content"]["assets"]:
                shutil.copyfile(ingress.FIXTURE_MANIFEST.parent / asset["file_name"], folder / asset["file_name"])
            path = self.write_manifest(folder, document)
            with patch.object(ingress, "authorization_errors", return_value=[]):
                with self.assertRaisesRegex(ingress.MrsqIngressError, "SHA-256"):
                    ingress.ingest(path, allow_synthetic=True)

    def test_direct_identifier_header_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "arbitrary.csv"
            path.write_text("Subject,E-mail\nsynthetic,none@example.invalid\n", encoding="utf-8")
            with self.assertRaisesRegex(ingress.MrsqIngressError, "direct-identifier"):
                ingress._read_rows(path)

    def test_manifest_schema_rejects_missing_source(self):
        document = self.manifest(); del document["source"]
        found = ingress.manifest_errors(document, ingress.load_json(ingress.POLICY))
        self.assertTrue(any("source" in item for item in found))

    def test_negative_numeric_value_is_rejected(self):
        with self.assertRaisesRegex(ingress.MrsqIngressError, "negative"):
            ingress._float({"x": "-1"}, "x")

    def test_missing_numeric_value_is_rejected(self):
        with self.assertRaisesRegex(ingress.MrsqIngressError, "missing or non-numeric"):
            ingress._float({}, "x")


if __name__ == "__main__":
    unittest.main()
