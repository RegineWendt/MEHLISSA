# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate PCQ-1.4 rights-aware pulmonary/capillary data ingress.

The manifest and path boundary are evaluated before a dataset is opened. The
command prints metadata summaries only; normalized observations remain inside
the returned Python object for a later locked evaluator.
"""

from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
from datetime import date
import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
POLICY = ROOT / "data/qualification/pulmonary-capillary-data-ingress-policy-v1.json"
POLICY_SCHEMA = ROOT / "data/schemas/pulmonary-capillary-data-ingress-policy/1.0.0.schema.json"
MANIFEST_SCHEMA = ROOT / "data/schemas/pulmonary-capillary-source-manifest/1.0.0.schema.json"
AMENDMENT = ROOT / "data/qualification/pulmonary-capillary-preoutcome-amendment-v1.json"
AMENDMENT_RELATIVE_PATH = "data/qualification/pulmonary-capillary-preoutcome-amendment-v1.json"
SYNTHETIC_ROOT = ROOT / "tests/data/pulmonary-capillary-ingress"

FIXTURES = {
    "pcq_hemodynamics": (
        SYNTHETIC_ROOT / "synthetic-hemodynamics-v1.manifest.json",
        SYNTHETIC_ROOT / "synthetic-hemodynamics-v1.json",
    ),
    "pcq_lobar_perfusion": (
        SYNTHETIC_ROOT / "synthetic-lobar-perfusion-v1.manifest.json",
        SYNTHETIC_ROOT / "synthetic-lobar-perfusion-v1.json",
    ),
    "pcq_capillary_volume": (
        SYNTHETIC_ROOT / "synthetic-capillary-volume-v1.manifest.json",
        SYNTHETIC_ROOT / "synthetic-capillary-volume-v1.json",
    ),
    "pcq_whole_pulmonary_transit": (
        SYNTHETIC_ROOT / "synthetic-whole-pulmonary-transit-v1.manifest.json",
        SYNTHETIC_ROOT / "synthetic-whole-pulmonary-transit-v1.json",
    ),
}


class DataIngressError(ValueError):
    """Raised when an ingress boundary, manifest, or dataset fails closed."""


@dataclass(frozen=True)
class IngressSummary:
    dataset_id: str
    measurement_family: str
    candidate_id: str
    participant_count: int
    normalized_record_count: int
    evidence_status: str
    measured_evidence: bool
    sample_status: str
    analysis_activation: str
    raw_observations_emitted: bool = False


@dataclass(frozen=True)
class IngressResult:
    summary: IngressSummary
    normalized_records: tuple[dict[str, Any], ...]


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def schema_errors(document: dict[str, Any], schema_path: Path) -> list[str]:
    validator = Draft202012Validator(load_json(schema_path), format_checker=FormatChecker())
    found: list[str] = []
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        found.append(f"{location}: {error.message}")
    return found


def inside(path: Path, parent: Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def family_map(policy: dict[str, Any]) -> dict[str, dict[str, Any]]:
    return {item["measurement_family"]: item for item in policy["families"]}


def policy_errors(document: dict[str, Any]) -> list[str]:
    result = schema_errors(document, POLICY_SCHEMA)
    if result:
        return result

    parent = document["policy"]["parent_amendment"]
    if parent["path"] != AMENDMENT_RELATIVE_PATH:
        result.append("parent amendment path changed from the frozen PCQ-1.3 amendment")
    parent_path = ROOT / parent["path"]
    if not parent_path.is_file() or sha256(parent_path) != parent["sha256"]:
        result.append("parent amendment is missing or its frozen SHA-256 changed")

    amendment = load_json(AMENDMENT)
    selected = {item["candidate_id"] for item in amendment["selected_sources"]}
    expected = {
        "pcq_hemodynamics": ("PCQ-H", "PCQ-SRC-H-001", ("PCQ-H1", "PCQ-H2", "PCQ-H3", "PCQ-H4"), "locked-awaiting-data", 3, 10),
        "pcq_lobar_perfusion": ("PCQ-R", "PCQ-SRC-R-002", ("PCQ-R1",), "locked-awaiting-feasibility-and-data", 5, 10),
        "pcq_capillary_volume": ("PCQ-C", "PCQ-SRC-CJ-001", ("PCQ-C1", "PCQ-C3", "PCQ-J1"), "locked-awaiting-data", 8, 12),
        "pcq_whole_pulmonary_transit": ("PCQ-C", "PCQ-SRC-C-003", ("PCQ-C2",), "blocked-observation-model", 0, 20),
    }
    actual: dict[str, tuple[Any, ...]] = {}
    for item in document["families"]:
        schema_path = ROOT / item["dataset_schema"]
        if not schema_path.is_file():
            result.append(f"dataset schema is missing: {item['dataset_schema']}")
        actual[item["measurement_family"]] = (
            item["track"],
            item["candidate_id"],
            tuple(item["endpoint_ids"]),
            item["activation_status"],
            item["pilot_floor"],
            item["decision_floor"],
        )
        if item["candidate_id"] not in selected:
            result.append(f"family selects a source absent from PCQ-1.3: {item['candidate_id']}")
    if actual != expected:
        result.append("family mappings, activation states, or sample floors changed from PCQ-1.3")

    forbidden = set(document["ingress_boundary"]["forbidden_field_names"])
    required_forbidden = {
        "name", "first_name", "last_name", "date_of_birth", "email", "address",
        "medical_record_number", "free_text_note", "source_system_identifier",
    }
    if not required_forbidden.issubset(forbidden):
        result.append("direct-identifier deny-list is incomplete")
    if document["next_step"]["increment"] != "PCQ-1.5":
        result.append("PCQ-1.5 must remain the next increment")
    full_text = json.dumps(document).lower()
    for phrase in ("no candidate participant-level outcome", "no external request", "clinical_use"):
        if phrase not in full_text:
            result.append(f"PCQ-1.4 safeguard is missing: {phrase}")
    return result


def validate_policy(path: Path = POLICY) -> dict[str, Any]:
    document = load_json(path)
    found = policy_errors(document)
    if found:
        raise DataIngressError("\n".join(found))
    return document


def manifest_errors(document: dict[str, Any], policy: dict[str, Any]) -> list[str]:
    result = schema_errors(document, MANIFEST_SCHEMA)
    if result:
        return result
    meta = document["manifest"]
    family = family_map(policy).get(meta["measurement_family"])
    if family is None:
        result.append(f"unknown measurement family: {meta['measurement_family']}")
        return result
    if meta["candidate_id"] != family["candidate_id"]:
        result.append("manifest candidate does not match the frozen family source")
    content = document["content"]
    if content["dataset_schema"] != family["dataset_schema"]:
        result.append("manifest dataset schema does not match the frozen family schema")
    schema_path = ROOT / content["dataset_schema"]
    if not schema_path.is_file() or sha256(schema_path) != content["dataset_schema_sha256"]:
        result.append("manifest dataset-schema SHA-256 is missing or incorrect")
    return result


def authorization_errors(
    manifest: dict[str, Any],
    dataset_path: Path,
    quarantine_root: Path | None,
    allow_synthetic: bool,
) -> list[str]:
    result: list[str] = []
    meta = manifest["manifest"]
    content = manifest["content"]
    rights = manifest["rights"]
    privacy = manifest["privacy"]
    independence = manifest["independence"]
    governance = manifest["governance"]

    if dataset_path.name != content["file_name"]:
        result.append("dataset filename does not match the manifest")
    if not dataset_path.is_file():
        result.append("dataset path is not an existing regular file")

    if meta["evidence_status"] == "synthetic_test_only":
        if not allow_synthetic:
            result.append("synthetic fixture requires the explicit --allow-synthetic flag")
        if not inside(dataset_path, SYNTHETIC_ROOT):
            result.append("synthetic fixture must remain inside the declared repository fixture root")
        expected = (
            rights["authorization_status"], rights["raw_redistribution"],
            privacy["repository_storage"], independence["status"],
            governance["quarantine_required"], governance["release_to_adapter"],
        )
        if expected != (
            "not_applicable_synthetic", "not_applicable_synthetic",
            "synthetic_fixture_allowed", "not_applicable_synthetic", False, True,
        ):
            result.append("synthetic fixture governance cannot masquerade as measured authorization")
        if not rights["processing_allowed"] or not rights["analysis_allowed"]:
            result.append("synthetic fixture must explicitly permit software-test processing")
    else:
        if inside(dataset_path, ROOT):
            result.append("measured participant data are forbidden inside the repository")
        if quarantine_root is None:
            result.append("measured participant data require an explicit quarantine root")
        else:
            if not quarantine_root.is_absolute():
                result.append("quarantine root must be absolute")
            if inside(quarantine_root, ROOT):
                result.append("quarantine root must be outside the repository")
            if not quarantine_root.is_dir():
                result.append("quarantine root must be an existing directory")
            if not inside(dataset_path, quarantine_root):
                result.append("measured dataset must resolve inside the explicit quarantine root")
        if rights["authorization_status"] != "approved":
            result.append("measured data authorization must be approved")
        if not rights["processing_allowed"] or not rights["analysis_allowed"]:
            result.append("measured processing and analysis must both be authorized")
        if privacy["repository_storage"] != "outside_repository_required":
            result.append("measured manifest must forbid repository storage")
        if independence["status"] != "confirmed_disjoint":
            result.append("measured validation cohort independence must be confirmed")
        if set(independence["source_cohort_ids"]) & set(independence["calibration_cohort_ids"]):
            result.append("source and calibration cohort identifiers overlap")
        if not governance["quarantine_required"] or not governance["release_to_adapter"]:
            result.append("measured data require quarantine and explicit release to the adapter")
        expires = rights.get("expires_on")
        if expires and date.fromisoformat(expires) < date.today():
            result.append("measured data authorization has expired")
    return result


def forbidden_fields(document: Any, denied: set[str], location: str = "<root>") -> list[str]:
    result: list[str] = []
    if isinstance(document, dict):
        for key, value in document.items():
            normalized = key.casefold().replace("-", "_").replace(" ", "_")
            child = f"{location}.{key}"
            if normalized in denied:
                result.append(f"forbidden direct-identifier field at {child}")
            result.extend(forbidden_fields(value, denied, child))
    elif isinstance(document, list):
        for index, value in enumerate(document):
            result.extend(forbidden_fields(value, denied, f"{location}[{index}]"))
    return result


def dataset_errors(
    document: dict[str, Any], manifest: dict[str, Any], policy: dict[str, Any]
) -> list[str]:
    meta = manifest["manifest"]
    schema_path = ROOT / manifest["content"]["dataset_schema"]
    result = schema_errors(document, schema_path)
    denied = set(policy["ingress_boundary"]["forbidden_field_names"])
    result.extend(forbidden_fields(document, denied))
    if result:
        return result

    dataset = document["dataset"]
    expected_origin = (
        "synthetic_test_fixture"
        if meta["evidence_status"] == "synthetic_test_only"
        else "measured_source"
    )
    if dataset["id"] != meta["dataset_id"]:
        result.append("dataset identifier does not match the manifest")
    if dataset["measurement_family"] != meta["measurement_family"]:
        result.append("dataset measurement family does not match the manifest")
    if dataset["candidate_id"] != meta["candidate_id"]:
        result.append("dataset candidate does not match the manifest")
    if dataset["data_origin"] != expected_origin:
        result.append("dataset origin does not match the manifest evidence status")
    participants = document["participants"]
    if len(participants) != manifest["content"]["participant_count"]:
        result.append("participant count does not match the manifest")
    participant_ids = [item["participant_id"] for item in participants]
    if len(participant_ids) != len(set(participant_ids)):
        result.append("participant pseudonyms must be unique")
    family = meta["measurement_family"]
    if family == "pcq_hemodynamics":
        for participant in participants:
            stages = participant["stages"]
            ordinals = [item["ordinal"] for item in stages]
            if ordinals != list(range(len(stages))):
                result.append(f"{participant['participant_id']}: stage ordinals must be contiguous from zero")
            if stages[0]["state"] != "rest" or stages[0]["workload_W"] != 0:
                result.append(f"{participant['participant_id']}: first stage must be zero-workload rest")
            if any(item["state"] != "exercise" for item in stages[1:]):
                result.append(f"{participant['participant_id']}: later stages must be exercise")
            workloads = [item["workload_W"] for item in stages]
            if workloads != sorted(workloads):
                result.append(f"{participant['participant_id']}: workload must not decrease")
            flows = [item["cardiac_output_L_min"] for item in stages]
            if max(flows) - min(flows) < 2.0:
                result.append(f"{participant['participant_id']}: cardiac-output span is below 2 L/min")
            for stage in stages:
                if stage["mean_pulmonary_arterial_pressure_mmHg"] <= stage["pulmonary_arterial_wedge_pressure_mmHg"]:
                    result.append(f"{participant['participant_id']}.{stage['id']}: mPAP must exceed PAWP")
                if "systolic_pulmonary_arterial_pressure_mmHg" in stage and stage["systolic_pulmonary_arterial_pressure_mmHg"] <= stage["diastolic_pulmonary_arterial_pressure_mmHg"]:
                    result.append(f"{participant['participant_id']}.{stage['id']}: systolic PAP must exceed diastolic PAP")
    elif family == "pcq_lobar_perfusion":
        for participant in participants:
            total = sum(participant["lobe_fractions"].values())
            if abs(total - 1.0) > 1.0e-9:
                result.append(f"{participant['participant_id']}: five lobe fractions must sum to one")
    elif family == "pcq_whole_pulmonary_transit":
        if document["observation_model"]["status"] != "blocked-observation-model":
            result.append("whole-pulmonary transit must retain its observation-model block")

    if meta["evidence_status"] == "synthetic_test_only":
        note = dataset["generation_note"].lower()
        if "arbitrary" not in note or "not derived" not in note:
            result.append("synthetic fixture must state that values are arbitrary and not derived from outcomes")
    return result


def adapt(document: dict[str, Any]) -> tuple[dict[str, Any], ...]:
    """Map a validated family document to one stable normalized record stream."""
    family = document["dataset"]["measurement_family"]
    records: list[dict[str, Any]] = []
    for participant in document["participants"]:
        common = {"participant_id": participant["participant_id"], "family": family}
        if family == "pcq_hemodynamics":
            for stage in participant["stages"]:
                records.append({
                    **common,
                    "stage_id": stage["id"],
                    "ordinal": stage["ordinal"],
                    "cardiac_output_L_min": stage["cardiac_output_L_min"],
                    "mPAP_mmHg": stage["mean_pulmonary_arterial_pressure_mmHg"],
                    "PAWP_mmHg": stage["pulmonary_arterial_wedge_pressure_mmHg"],
                })
        elif family == "pcq_lobar_perfusion":
            records.append({**common, "lobe_fractions": dict(participant["lobe_fractions"])})
        elif family == "pcq_capillary_volume":
            records.append({
                **common,
                "functional_capillary_volume_mL": participant["functional_capillary_volume_mL"],
                "cardiac_output_L_min": participant["cardiac_output_L_min"],
                "hemoglobin_g_dL": participant["hemoglobin_g_dL"],
            })
        else:
            records.append({
                **common,
                "whole_pulmonary_transit_s": participant["mean_bolus_transit_time_s"],
                "analysis_activation": "blocked-observation-model",
            })
    return tuple(records)


def ingest(
    manifest_path: Path,
    dataset_path: Path,
    *,
    quarantine_root: Path | None = None,
    allow_synthetic: bool = False,
) -> IngressResult:
    """Authorize first, then read, validate, and normalize one dataset."""
    policy = validate_policy()
    manifest = load_json(manifest_path)
    found = manifest_errors(manifest, policy)
    if found:
        raise DataIngressError("\n".join(found))

    # This boundary deliberately precedes load_json(dataset_path).
    found = authorization_errors(manifest, dataset_path, quarantine_root, allow_synthetic)
    if found:
        raise DataIngressError("\n".join(found))

    if sha256(dataset_path) != manifest["content"]["sha256"]:
        raise DataIngressError("dataset SHA-256 does not match the authorized manifest")
    document = load_json(dataset_path)
    found = dataset_errors(document, manifest, policy)
    if found:
        raise DataIngressError("\n".join(found))

    records = adapt(document)
    meta = manifest["manifest"]
    family = family_map(policy)[meta["measurement_family"]]
    count = len(document["participants"])
    if family["activation_status"] == "blocked-observation-model":
        sample_status = "blocked-observation-model"
    elif count >= family["decision_floor"]:
        sample_status = "decision-sample-floor-met"
    elif family["pilot_floor"] and count >= family["pilot_floor"]:
        sample_status = "bounded-pilot-sample"
    else:
        sample_status = "inconclusive-insufficient-sample"
    measured = meta["evidence_status"] == "measured_validation"
    activation = family["activation_status"] if measured else "software-test-only"
    summary = IngressSummary(
        dataset_id=meta["dataset_id"],
        measurement_family=meta["measurement_family"],
        candidate_id=meta["candidate_id"],
        participant_count=count,
        normalized_record_count=len(records),
        evidence_status=meta["evidence_status"],
        measured_evidence=measured,
        sample_status=sample_status,
        analysis_activation=activation,
    )
    return IngressResult(summary=summary, normalized_records=records)


def validate_fixtures() -> list[IngressSummary]:
    summaries: list[IngressSummary] = []
    for manifest_path, dataset_path in FIXTURES.values():
        summaries.append(ingest(manifest_path, dataset_path, allow_synthetic=True).summary)
    return summaries


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--data", type=Path)
    parser.add_argument("--quarantine-root", type=Path)
    parser.add_argument("--allow-synthetic", action="store_true")
    arguments = parser.parse_args()
    try:
        if bool(arguments.manifest) != bool(arguments.data):
            raise DataIngressError("--manifest and --data must be supplied together")
        if arguments.manifest:
            result = ingest(
                arguments.manifest,
                arguments.data,
                quarantine_root=arguments.quarantine_root,
                allow_synthetic=arguments.allow_synthetic,
            )
            print(json.dumps(asdict(result.summary), indent=2, sort_keys=True))
        else:
            summaries = validate_fixtures()
            record_count = sum(item.normalized_record_count for item in summaries)
            print(
                "PCQ-1.4 data ingress: ok "
                f"({len(summaries)} outcome-blind synthetic families, "
                f"{record_count} normalized records, no measured outcomes acquired)"
            )
    except (OSError, json.JSONDecodeError, DataIngressError) as error:
        print(f"PCQ-1.4 data ingress: FAILED\n{error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
