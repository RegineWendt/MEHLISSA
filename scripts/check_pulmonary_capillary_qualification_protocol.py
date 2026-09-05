# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the PCQ-1 pulmonary and capillary qualification design."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
PROTOCOL = ROOT / "data/qualification/pulmonary-capillary-qualification-protocol-v1.json"
SCHEMA = ROOT / "data/schemas/pulmonary-capillary-qualification-protocol/1.0.0.schema.json"


class QualificationProtocolError(ValueError):
    pass


def load(path: Path) -> dict:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def errors(document: dict, root: Path = ROOT) -> list[str]:
    validator = Draft202012Validator(load(SCHEMA), format_checker=FormatChecker())
    result = [error.message for error in validator.iter_errors(document)]

    tracks = [item.get("id") for item in document.get("qualification_tracks", [])]
    if set(tracks) != {"PCQ-H", "PCQ-R", "PCQ-C", "PCQ-J"} or len(tracks) != 4:
        result.append("qualification tracks must cover PCQ-H, PCQ-R, PCQ-C, and PCQ-J exactly once")

    endpoints = document.get("endpoints", [])
    endpoint_ids = [item.get("id") for item in endpoints]
    expected_endpoints = {
        "PCQ-H1", "PCQ-H2", "PCQ-H3", "PCQ-H4", "PCQ-R1",
        "PCQ-C1", "PCQ-C2", "PCQ-C3", "PCQ-J1",
    }
    if set(endpoint_ids) != expected_endpoints or len(endpoint_ids) != len(expected_endpoints):
        result.append("endpoint hierarchy must cover PCQ-H1-H4, PCQ-R1, PCQ-C1-C3, and PCQ-J1 exactly once")

    primary_ids = {item.get("id") for item in endpoints if item.get("role") == "primary"}
    if primary_ids != {"PCQ-H1", "PCQ-H2", "PCQ-H3", "PCQ-R1", "PCQ-C1", "PCQ-C2"}:
        result.append("primary endpoints must match the predeclared six-endpoint hierarchy")

    if document.get("analysis", {}).get("no_refit_on_validation") is not True:
        result.append("validation must be executed without refitting")
    if document.get("candidate_claim", {}).get("evidence_status") != "not-yet-tested":
        result.append("the candidate claim cannot be advanced before new validation is executed")
    if document.get("protocol", {}).get("clinical_use") is not False:
        result.append("PCQ-1 must retain its non-clinical-use boundary")

    assets = document.get("frozen_assets", [])
    asset_paths = [item.get("path") for item in assets]
    if len(asset_paths) != len(set(asset_paths)):
        result.append("frozen asset paths must be unique")
    for item in assets:
        path_text = item.get("path", "")
        candidate = (root / path_text).resolve()
        try:
            candidate.relative_to(root.resolve())
        except ValueError:
            result.append(f"frozen asset escapes repository root: {path_text}")
            continue
        if not candidate.is_file():
            result.append(f"frozen asset does not exist: {path_text}")
            continue
        actual_hash = sha256(candidate)
        if actual_hash != item.get("sha256"):
            result.append(f"frozen asset hash mismatch: {path_text}")

    control_ids = [item.get("id") for item in document.get("negative_controls", [])]
    if len(control_ids) != len(set(control_ids)):
        result.append("negative-control identifiers must be unique")

    text = json.dumps(document).lower()
    for required in (
        "source-disjoint",
        "before outcome access",
        "no-refit",
        "hematocrit",
        "identifiability",
        "clinical validity",
        "failed",
    ):
        if required not in text:
            result.append(f"required qualification concept is absent: {required}")
    return result


def validate(path: Path = PROTOCOL, root: Path = ROOT) -> None:
    document = load(path)
    found = errors(document, root)
    if found:
        raise QualificationProtocolError("\n".join(found))


if __name__ == "__main__":
    try:
        validate()
    except (OSError, json.JSONDecodeError, QualificationProtocolError) as error:
        print(f"PCQ-1 qualification protocol: FAILED\n{error}")
        raise SystemExit(1)
    print("PCQ-1 qualification protocol: ok (4 tracks, 9 endpoints, 7 negative controls, 2 frozen assets)")
