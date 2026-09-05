# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

import check_pulmonary_capillary_data_ingress as ingress  # noqa: E402


class PulmonaryCapillaryDataIngressTests(unittest.TestCase):
    def setUp(self) -> None:
        self.policy = ingress.load_json(ingress.POLICY)

    def fixture(self, family: str) -> tuple[Path, Path, dict, dict]:
        manifest_path, data_path = ingress.FIXTURES[family]
        return (
            manifest_path,
            data_path,
            ingress.load_json(manifest_path),
            ingress.load_json(data_path),
        )

    def write_json(self, directory: Path, name: str, document: dict) -> Path:
        path = directory / name
        path.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
        return path

    def test_policy_and_all_four_synthetic_adapters_are_valid(self) -> None:
        ingress.validate_policy()
        summaries = ingress.validate_fixtures()
        self.assertEqual(4, len(summaries))
        self.assertEqual(
            {
                "pcq_hemodynamics",
                "pcq_lobar_perfusion",
                "pcq_capillary_volume",
                "pcq_whole_pulmonary_transit",
            },
            {item.measurement_family for item in summaries},
        )
        self.assertTrue(all(not item.measured_evidence for item in summaries))
        self.assertTrue(all(not item.raw_observations_emitted for item in summaries))

    def test_synthetic_data_require_explicit_test_authorization(self) -> None:
        manifest_path, data_path, _, _ = self.fixture("pcq_hemodynamics")
        with self.assertRaisesRegex(ingress.DataIngressError, "explicit --allow-synthetic"):
            ingress.ingest(manifest_path, data_path)

    def test_measured_data_inside_repository_are_rejected_before_open(self) -> None:
        _, data_path, manifest, _ = self.fixture("pcq_hemodynamics")
        manifest["manifest"]["evidence_status"] = "measured_validation"
        manifest["rights"].update(
            authorization_status="approved",
            approval_reference="institutional-test-reference",
            raw_redistribution="prohibited",
        )
        manifest["privacy"]["repository_storage"] = "outside_repository_required"
        manifest["independence"].update(
            status="confirmed_disjoint",
            source_cohort_ids=["source-cohort"],
            calibration_cohort_ids=["calibration-cohort"],
        )
        manifest["governance"].update(quarantine_required=True, release_to_adapter=True)
        with tempfile.TemporaryDirectory(prefix="mehlissa-pcq-manifest-") as directory:
            manifest_path = self.write_json(Path(directory), "measured.manifest.json", manifest)
            real_load = ingress.load_json

            def guarded_load(path: Path) -> dict:
                if Path(path).resolve() == data_path.resolve():
                    raise AssertionError("dataset was opened before quarantine authorization")
                return real_load(path)

            with mock.patch.object(ingress, "load_json", side_effect=guarded_load):
                with self.assertRaisesRegex(ingress.DataIngressError, "forbidden inside the repository"):
                    ingress.ingest(manifest_path, data_path)

    def test_pending_or_expired_rights_and_cohort_overlap_fail_closed(self) -> None:
        _, data_path, manifest, _ = self.fixture("pcq_capillary_volume")
        manifest["manifest"]["evidence_status"] = "measured_validation"
        manifest["rights"].update(
            authorization_status="pending",
            raw_redistribution="prohibited",
        )
        manifest["privacy"]["repository_storage"] = "outside_repository_required"
        manifest["independence"].update(
            status="confirmed_disjoint",
            source_cohort_ids=["same-cohort"],
            calibration_cohort_ids=["same-cohort"],
        )
        manifest["governance"].update(quarantine_required=True, release_to_adapter=True)
        found = ingress.authorization_errors(manifest, data_path, None, False)
        self.assertTrue(any("authorization must be approved" in item for item in found))
        self.assertTrue(any("cohort identifiers overlap" in item for item in found))

        manifest["rights"].update(
            authorization_status="approved",
            approval_reference="institutional-test-reference",
            expires_on="2000-01-01",
        )
        found = ingress.authorization_errors(manifest, data_path, None, False)
        self.assertTrue(any("authorization has expired" in item for item in found))

    def test_checksum_and_schema_identity_are_enforced(self) -> None:
        _, data_path, manifest, _ = self.fixture("pcq_lobar_perfusion")
        manifest["content"]["sha256"] = "0" * 64
        with tempfile.TemporaryDirectory(prefix="mehlissa-pcq-manifest-") as directory:
            manifest_path = self.write_json(Path(directory), "bad-hash.manifest.json", manifest)
            with self.assertRaisesRegex(ingress.DataIngressError, "dataset SHA-256"):
                ingress.ingest(manifest_path, data_path, allow_synthetic=True)
        manifest["content"]["dataset_schema_sha256"] = "0" * 64
        self.assertTrue(
            any("dataset-schema SHA-256" in item for item in ingress.manifest_errors(manifest, self.policy))
        )

    def test_age_units_stage_tuple_and_direct_identifiers_fail_closed(self) -> None:
        _, _, manifest, document = self.fixture("pcq_hemodynamics")
        document["participants"][0]["age_years"] = 41
        document["units"]["pressure"] = "Pa"
        document["participants"][0]["stages"][1].pop("pulmonary_arterial_wedge_pressure_mmHg")
        document["participants"][0]["name"] = "forbidden"
        found = ingress.dataset_errors(document, manifest, self.policy)
        joined = "\n".join(found)
        self.assertIn("greater than the maximum of 40", joined)
        self.assertIn("'mmHg' was expected", joined)
        self.assertIn("required property", joined)
        self.assertIn("forbidden direct-identifier field", joined)

    def test_stage_order_flow_span_and_lobe_normalization_are_semantic_gates(self) -> None:
        _, _, h_manifest, hemodynamics = self.fixture("pcq_hemodynamics")
        hemodynamics["participants"][0]["stages"][1]["ordinal"] = 2
        hemodynamics["participants"][0]["stages"][2]["cardiac_output_L_min"] = 6.0
        h_found = ingress.dataset_errors(hemodynamics, h_manifest, self.policy)
        self.assertTrue(any("ordinals" in item for item in h_found))
        self.assertTrue(any("span" in item for item in h_found))

        _, _, r_manifest, regional = self.fixture("pcq_lobar_perfusion")
        regional["participants"][0]["lobe_fractions"]["left_lower"] = 0.20
        r_found = ingress.dataset_errors(regional, r_manifest, self.policy)
        self.assertTrue(any("sum to one" in item for item in r_found))

    def test_transit_adapter_cannot_remove_observation_model_block(self) -> None:
        manifest_path, data_path, manifest, document = self.fixture("pcq_whole_pulmonary_transit")
        result = ingress.ingest(manifest_path, data_path, allow_synthetic=True)
        self.assertEqual("blocked-observation-model", result.summary.sample_status)
        self.assertEqual("software-test-only", result.summary.analysis_activation)
        document["observation_model"]["status"] = "ready"
        found = ingress.dataset_errors(document, manifest, self.policy)
        self.assertTrue(any("blocked-observation-model" in item for item in found))

    def test_policy_parent_hash_and_frozen_family_mapping_are_guarded(self) -> None:
        document = copy.deepcopy(self.policy)
        document["policy"]["parent_amendment"]["sha256"] = "0" * 64
        self.assertTrue(any("parent amendment" in item for item in ingress.policy_errors(document)))
        document = copy.deepcopy(self.policy)
        document["policy"]["parent_amendment"]["path"] = "data/qualification/other.json"
        self.assertTrue(any("parent amendment path" in item for item in ingress.policy_errors(document)))
        document = copy.deepcopy(self.policy)
        document["families"][0]["decision_floor"] = 5
        self.assertTrue(any("sample floors" in item for item in ingress.policy_errors(document)))


if __name__ == "__main__":
    unittest.main()
