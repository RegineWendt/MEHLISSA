# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the outcome-blind PCQ-1.5a repository-first data audit."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
AUDIT = ROOT / "data/qualification/pulmonary-capillary-repository-audit-v1.json"
SCHEMA = ROOT / "data/schemas/pulmonary-capillary-repository-audit/1.0.0.schema.json"
PARENT_RELATIVE_PATH = "data/qualification/pulmonary-capillary-evidence-candidate-register-v1.json"
EXPECTED_PARENT_SHA256 = "40f94e63abcb13e9d292efec3e38270a61879107890e6c6fb03fe782cc98cb14"
EXPECTED_BASELINE = "d3bd0024c344e6b61f55e013c5c76d6aa509ed0f"


class RepositoryAuditError(ValueError):
    """Raised when the checked repository-first audit is inconsistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def errors(document: dict[str, Any]) -> list[str]:
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    result = []
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"{location}: {error.message}")
    if result:
        return result

    header = document["audit"]
    parent = header["parent_register"]
    parent_path = ROOT / parent["path"]
    if parent["path"] != PARENT_RELATIVE_PATH:
        result.append("parent PCQ-1.2 register path changed")
    if (
        parent["sha256"] != EXPECTED_PARENT_SHA256
        or not parent_path.is_file()
        or sha256(parent_path) != parent["sha256"]
    ):
        result.append("parent PCQ-1.2 register hash is missing or changed")
    if header["baseline_commit"] != EXPECTED_BASELINE:
        result.append("PCQ-1.5a baseline commit changed")

    targets = document["target_studies"]
    target_ids = [item["candidate_id"] for item in targets]
    if len(target_ids) != len(set(target_ids)):
        result.append("target candidate identifiers must be unique")
    required_targets = {
        "PCQ-SRC-H-001",
        "PCQ-SRC-R-002",
        "PCQ-SRC-CJ-001",
        "PCQ-SRC-C-002",
        "PCQ-SRC-C-003",
    }
    if set(target_ids) != required_targets:
        result.append("repository audit must cover the five frozen priority or backup targets")
    by_target = {item["candidate_id"]: item for item in targets}
    arizona = by_target.get("PCQ-SRC-H-001", {})
    if arizona.get("repository_status") != "software-only":
        result.append("Arizona public repository object must remain classified as software-only")
    if not any(item.get("resource_type") == "software" for item in arizona.get("objects", [])):
        result.append("Arizona software repository object is missing")
    lassen = by_target.get("PCQ-SRC-C-003", {})
    if lassen.get("repository_status") != "supplements-only":
        result.append("Lassen public objects must remain classified as supplements-only")

    alternatives = document["alternative_sources"]
    alternative_ids = [item["id"] for item in alternatives]
    if len(alternative_ids) != len(set(alternative_ids)):
        result.append("alternative source identifiers must be unique")
    required_alternatives = {
        "PCQ-REPO-AUX-001",
        "PCQ-REPO-AUX-002",
        "PCQ-REPO-AUX-003",
        "PCQ-REPO-AUX-004",
        "PCQ-REPO-AUX-005",
    }
    if set(alternative_ids) != required_alternatives:
        result.append("repository audit alternative-source set changed")
    if any(item["file_inspection"] != "not-opened-or-downloaded" for item in alternatives):
        result.append("alternative participant files must remain unopened and undownloaded")
    if any(item["eligibility"] not in {"supplementary-candidate-only", "later-stress-test-only", "not-eligible-for-current-pcq"} for item in alternatives):
        result.append("an alternative source was silently promoted to a primary role")

    decision = document["decision"]
    if any(
        decision[key]
        for key in (
            "drop_in_repository_dataset_found",
            "participant_files_opened",
            "participant_outcomes_inspected",
            "external_requests_sent",
            "frozen_source_ranking_changed",
        )
    ):
        result.append("repository audit must not claim data access, contact, ranking change, or a drop-in dataset")
    queue = decision["contact_queue"]
    priorities = [item["priority"] for item in queue]
    if priorities != list(range(1, len(queue) + 1)):
        result.append("contact priorities must be contiguous and ordered from one")
    if [item["candidate_id"] for item in queue] != [
        "PCQ-SRC-CJ-001",
        "PCQ-SRC-H-001",
        "PCQ-SRC-R-002",
        "PCQ-SRC-C-003",
    ]:
        result.append("contact queue must retain D'Souza, Arizona, Bailey, then Lassen")

    full_text = json.dumps(document).lower()
    for phrase in (
        "no newly located participant file",
        "not a replacement",
        "successor pre-outcome amendment",
        "rights-aware ingress",
        "clinical_use",
    ):
        if phrase not in full_text:
            result.append(f"required repository-audit safeguard is absent: {phrase}")
    return result


def validate(path: Path = AUDIT) -> dict[str, Any]:
    document = load_json(path)
    found = errors(document)
    if found:
        raise RepositoryAuditError("\n".join(found))
    return document


def main() -> int:
    try:
        document = validate()
        print(
            "PCQ-1.5a repository-first data audit: ok "
            f"({len(document['target_studies'])} targets, "
            f"{len(document['alternative_sources'])} alternatives, "
            "no participant files opened, no source roles changed)"
        )
    except (OSError, json.JSONDecodeError, RepositoryAuditError) as error:
        print(f"PCQ-1.5a repository-first data audit: FAILED\n{error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
