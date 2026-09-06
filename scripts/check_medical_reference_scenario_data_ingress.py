# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Qualify the MRSQ-1.3 manifest-first CSV boundary on synthetic data.

Authorization and path checks deliberately happen before any source CSV is
opened. Participant-level output is never printed or returned.
"""

from __future__ import annotations

import csv
import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker

ROOT = Path(__file__).resolve().parents[1]
POLICY = ROOT / "data/qualification/medical-reference-scenario-data-ingress-policy-v1.json"
POLICY_SCHEMA = ROOT / "data/schemas/medical-reference-scenario-data-ingress-policy/1.0.0.schema.json"
MANIFEST_SCHEMA = ROOT / "data/schemas/medical-reference-scenario-source-manifest/1.0.0.schema.json"
FIXTURE_MANIFEST = ROOT / "tests/data/mrsq1-ingress/synthetic.manifest.json"
DIRECT_IDENTIFIER_NORMALIZED = {
    "name", "firstname", "lastname", "givenname", "familyname", "email",
    "telephone", "phone", "address", "street", "postcode", "postalcode",
    "dateofbirth", "birthdate", "medicalrecordnumber",
}


class MrsqIngressError(ValueError):
    """Raised when MRSQ ingress must fail closed."""


@dataclass(frozen=True)
class IngressSummary:
    run_id: str
    evidence_status: str
    participant_count: int
    aortic_frame_count: int
    organ_frame_count: int
    region_count: int
    source_values_exposed: bool = False


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _schema_errors(document: dict[str, Any], schema: dict[str, Any]) -> list[str]:
    validator = Draft202012Validator(schema, format_checker=FormatChecker())
    return [f"{'.'.join(map(str, error.path)) or '<root>'}: {error.message}"
            for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path))]


def policy_errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    result = _schema_errors(document, load_json(root / POLICY_SCHEMA.relative_to(ROOT)))
    if result:
        return result
    parent = document["policy"]["parent_protocol"]
    parent_path = root / parent["path"]
    if not parent_path.is_file() or sha256(parent_path) != parent["sha256"]:
        result.append("frozen parent protocol is missing or has changed")
    gates = {item["id"]: item["status"] for item in document["readiness_gates"]}
    required_blocked = {"local-institutional-determination", "quarantine-retention-access",
                        "download-content-sha256", "exact-frame-duration-authority",
                        "measured-ingress-release"}
    if any(gates.get(item) != "BLOCKED" for item in required_blocked):
        result.append("measured ingress blockers may not be silently released")
    if document["decision"]["measured_ingress"] != "BLOCKED":
        result.append("measured ingress must remain blocked in policy v1")
    return result


def manifest_errors(document: dict[str, Any], policy: dict[str, Any], root: Path = ROOT) -> list[str]:
    result = _schema_errors(document, load_json(root / MANIFEST_SCHEMA.relative_to(ROOT)))
    if result:
        return result
    protocol = document["protocol"]
    parent = policy["policy"]["parent_protocol"]
    if protocol["path"] != parent["path"] or protocol["sha256"] != parent["sha256"]:
        result.append("manifest protocol identity differs from the frozen policy")
    if protocol["data_revision"] != policy["source_contract"]["revision"]:
        result.append("manifest data revision differs from the frozen revision")
    roles = [item["role"] for item in document["content"]["assets"]]
    if sorted(roles) != sorted(["administration", "aortic_input", "organ_tac", "region_volume"]):
        result.append("manifest must contain each of the four frozen roles exactly once")
    if document["semantics"]["region_components"] != policy["source_contract"]["region_components"]:
        result.append("manifest region mapping differs from the frozen policy")
    return result


def authorization_errors(manifest: dict[str, Any], manifest_path: Path, policy: dict[str, Any],
                         quarantine_root: Path | None, allow_synthetic: bool) -> list[str]:
    result: list[str] = []
    evidence = manifest["manifest"]["evidence_status"]
    governance = manifest["governance"]
    if evidence == "synthetic_test_only":
        if not allow_synthetic:
            result.append("synthetic ingress requires explicit allow_synthetic")
        if manifest["source"]["provider"] != "synthetic":
            result.append("synthetic manifest must identify a synthetic source")
        if manifest_path.resolve().parent != (ROOT / "tests/data/mrsq1-ingress").resolve():
            result.append("synthetic fixture manifest must remain in the declared fixture root")
    else:
        # Policy v1 intentionally cannot release real rows. This check happens
        # before asset paths are resolved or any CSV is opened.
        result.append("measured ingress is blocked by MRSQ-1.3 policy v1")
        if governance["local_determination"] != "approved-for-open-deidentified-research-reuse":
            result.append("approved local institutional determination is absent")
        if not governance["quarantine_required"] or not governance["release_to_adapter"]:
            result.append("quarantine/release governance gate is not approved")
        if quarantine_root is None or not quarantine_root.is_absolute() or not quarantine_root.is_dir():
            result.append("an absolute existing quarantine root is required")
        elif ROOT.resolve() == quarantine_root.resolve() or ROOT.resolve() in quarantine_root.resolve().parents:
            result.append("quarantine root must be outside the repository")
    return result


def _normal(column: str) -> str:
    return "".join(character.lower() for character in column if character.isalnum())


def _read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8-sig") as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames is None:
            raise MrsqIngressError("CSV has no header")
        unexpected = DIRECT_IDENTIFIER_NORMALIZED.intersection(_normal(item) for item in reader.fieldnames)
        if unexpected:
            raise MrsqIngressError("CSV contains a forbidden direct-identifier column")
        return list(reader)


def _float(row: dict[str, str], field: str) -> float:
    try:
        value = float(row[field])
    except (KeyError, ValueError) as error:
        raise MrsqIngressError(f"missing or non-numeric required field: {field}") from error
    if value < 0.0:
        raise MrsqIngressError(f"negative required field: {field}")
    return value


def ingest(manifest_path: Path = FIXTURE_MANIFEST, *, quarantine_root: Path | None = None,
           allow_synthetic: bool = False, root: Path = ROOT) -> IngressSummary:
    policy = load_json(root / POLICY.relative_to(ROOT))
    found = policy_errors(policy, root)
    if found:
        raise MrsqIngressError("; ".join(found))
    manifest = load_json(manifest_path)
    found = manifest_errors(manifest, policy, root)
    if found:
        raise MrsqIngressError("; ".join(found))
    found = authorization_errors(manifest, manifest_path, policy, quarantine_root, allow_synthetic)
    if found:
        raise MrsqIngressError("; ".join(found))

    # No source CSV may be opened before the complete authorization boundary above.
    base = manifest_path.resolve().parent if manifest["manifest"]["evidence_status"] == "synthetic_test_only" else quarantine_root
    assert base is not None
    assets: dict[str, Path] = {}
    for asset in manifest["content"]["assets"]:
        path = (base / asset["file_name"]).resolve()
        if base.resolve() not in path.parents:
            raise MrsqIngressError("asset path escapes its authorized root")
        if not path.is_file() or path.stat().st_size != asset["size_bytes"] or sha256(path) != asset["sha256"]:
            raise MrsqIngressError("asset size or SHA-256 differs from the authorized manifest")
        assets[asset["role"]] = path

    metadata = _read_rows(assets["administration"])
    inputs = _read_rows(assets["aortic_input"])
    tacs = _read_rows(assets["organ_tac"])
    volumes = _read_rows(assets["region_volume"])
    required_curve = {"Subject", "Task", "Label Name", "Erosion Iterations", "Frame Index",
                      "Frame Time Middle [s]", "Frame Duration [s]", "PET Mean [Bq/mL]"}
    for rows, name in ((inputs, "aortic input"), (tacs, "organ TAC")):
        if not rows or not required_curve.issubset(rows[0]):
            raise MrsqIngressError(f"{name} lacks the exact frame-duration contract")
        for row in rows:
            if _float(row, "Frame Duration [s]") <= 0.0:
                raise MrsqIngressError("frame duration must be positive")
            _float(row, "PET Mean [Bq/mL]")
    if not metadata or not {"Subject", "Injected Activity [MBq]", "Weight [kg]"}.issubset(metadata[0]):
        raise MrsqIngressError("administration table lacks frozen fields")
    if not volumes or not {"Subject", "Label Name", "Volume [mL]"}.issubset(volumes[0]):
        raise MrsqIngressError("volume table lacks frozen fields")
    for row in metadata:
        _float(row, "Injected Activity [MBq]"); _float(row, "Weight [kg]")
    for row in volumes:
        _float(row, "Volume [mL]")
    subjects = {row["Subject"] for row in metadata}
    if subjects != {row["Subject"] for row in inputs} or subjects != {row["Subject"] for row in tacs} or subjects != {row["Subject"] for row in volumes}:
        raise MrsqIngressError("source tables do not have the same participant linkage set")
    selector = policy["source_contract"]["aortic_selector"]
    selected_inputs = [row for row in inputs if row["Task"].startswith(selector["task_prefix"])
                       and row["Label Name"] == selector["label_name"]
                       and int(row["Erosion Iterations"]) == selector["erosion_iterations"]]
    if not selected_inputs:
        raise MrsqIngressError("frozen aortic selector produced no frames")
    expected_regions = {label for labels in policy["source_contract"]["region_components"].values() for label in labels}
    if expected_regions - {row["Label Name"] for row in tacs} or expected_regions - {row["Label Name"] for row in volumes}:
        raise MrsqIngressError("one or more frozen region components are missing")
    # Opaque linkage is exercised without returning identifiers.
    for subject in subjects:
        hashlib.sha256((manifest["manifest"]["run_linkage_salt"] + subject).encode()).hexdigest()[:16]
    return IngressSummary(manifest["manifest"]["run_id"], manifest["manifest"]["evidence_status"],
                          len(subjects), len(selected_inputs), len(tacs), len(expected_regions))


def main() -> int:
    try:
        summary = ingest(allow_synthetic=True)
    except (OSError, json.JSONDecodeError, MrsqIngressError) as error:
        print(f"MRSQ-1.3 selective ingress: FAILED\n{error}")
        return 1
    print("MRSQ-1.3 selective ingress: synthetic PASS, measured BLOCKED "
          f"({summary.participant_count} arbitrary subject, {summary.aortic_frame_count} aortic frames, "
          f"{summary.region_count} source region labels; no participant files opened)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
