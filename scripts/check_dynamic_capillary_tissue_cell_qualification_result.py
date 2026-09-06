# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Independently verify the archived DCCQ-1.4 through DCCQ-1.7 result."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
RESULT = ROOT / "data/qualification/dynamic-capillary-tissue-cell-qualification-result-v1.json"
SCHEMA = ROOT / "data/schemas/dynamic-capillary-tissue-cell-qualification-result/1.0.0.schema.json"
HASH_POLICY = "SHA-256 over Git-canonical LF bytes for text artifacts; binary bytes unchanged"
TEXT_SUFFIXES = {".csv", ".json", ".md", ".txt", ".cpp", ".hpp"}
OWNER_COLUMNS = [
    "blood_free_mol", "endothelium_free_mol", "interstitium_free_mol",
    "receptor_bound_mol", "internalized_mol", "cleared_or_degraded_mol",
    "cumulative_outlet_mol",
]
CSV_COLUMNS = [
    "time_s", "initial_mol", "cumulative_inlet_mol", *OWNER_COLUMNS,
    "occupancy_fraction", "applied_feedback_multiplier",
    "scheduled_feedback_multiplier", "balance_error_mol",
]
EXPECTED_GATE_STATUS = {
    "DCCQ-G1": "PASS",
    "DCCQ-G2": "PASS",
    "DCCQ-G3": "PASS",
    "DCCQ-G4": "PASS",
    "DCCQ-G5": "PASS",
    "DCCQ-G6": "PARTIAL",
    "DCCQ-G7": "BLOCKED",
    "DCCQ-G8": "PARTIAL",
}
EXPECTED_INCREMENT_STATUS = {
    "DCCQ-1.3": "COMPLETE",
    "DCCQ-1.4": "COMPLETE",
    "DCCQ-1.5": "COMPLETE",
    "DCCQ-1.6": "COMPLETE_WITH_G6_PARTIAL_AND_G7_BLOCKED",
    "DCCQ-1.7": "PARTIAL_EXTERNAL_HUMAN_REVIEW_BLOCKED",
}
DOCUMENTATION = {
    "docs/qualification/DCCQ1_QUALIFICATION_RESULT.md": "DCCQ-G7",
    "docs/USER_GUIDE.md": "DCCQ-1.7",
    "docs/DEVELOPMENT.md": "DynamicCapillaryTissueCellModel",
    "docs/architecture/SOFTWARE_ARCHITECTURE.md": "seven-owner",
    "docs/ROADMAP.md": "DCCQ-1.7",
    "docs/requirements/TRACEABILITY_MATRIX.md": "DCCQ-G5",
    "docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md": "41 trajectories",
}


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def canonical_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES:
        data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    return data


def sha256(path: Path) -> str:
    return hashlib.sha256(canonical_bytes(path)).hexdigest()


def repository_path(root: Path, relative: str, found: list[str]) -> Path | None:
    path = (root / relative).resolve()
    try:
        path.relative_to(root.resolve())
    except ValueError:
        found.append(f"repository path escapes root: {relative}")
        return None
    return path


def read_trajectory(path: Path, found: list[str]) -> list[dict[str, float]]:
    try:
        with path.open(encoding="utf-8", newline="") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != CSV_COLUMNS:
                found.append(f"trajectory columns changed: {path.name}")
                return []
            rows = [{key: float(value) for key, value in row.items()} for row in reader]
    except (OSError, TypeError, ValueError) as error:
        found.append(f"cannot parse trajectory {path.name}: {error}")
        return []
    if not rows or not all(math.isfinite(value) for row in rows for value in row.values()):
        found.append(f"trajectory is empty or non-finite: {path.name}")
    return rows


def final_error(left: list[dict[str, float]], right: list[dict[str, float]]) -> float:
    return sum(abs(left[-1][name] - right[-1][name]) for name in OWNER_COLUMNS) / left[0][
        "initial_mol"
    ]


def close(left: float, right: float, tolerance: float = 1.0e-12) -> bool:
    return math.isclose(left, right, rel_tol=tolerance, abs_tol=1.0e-30)


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    found: list[str] = []
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        found.append(f"{location}: {error.message}")
    if found:
        return found

    protocol_path = repository_path(root, document["protocol"]["path"], found)
    if protocol_path is None or not protocol_path.is_file():
        found.append("frozen protocol is missing")
        return found
    if sha256(protocol_path) != document["protocol"]["sha256"]:
        found.append("frozen protocol hash changed")
    protocol = load_json(protocol_path)
    for asset in protocol["frozen_implementation"]:
        path = repository_path(root, asset["path"], found)
        if path is None or not path.is_file() or sha256(path) != asset["sha256"]:
            found.append(f"frozen implementation changed: {asset['path']}")

    archive = repository_path(root, document["archive"]["path"], found)
    manifest_path = repository_path(root, document["archive"]["checksum_manifest"]["path"], found)
    if archive is None or not archive.is_dir() or manifest_path is None or not manifest_path.is_file():
        found.append("qualification archive or manifest is missing")
        return found
    if sha256(manifest_path) != document["archive"]["checksum_manifest"]["sha256"]:
        found.append("qualification manifest hash changed")
        return found
    manifest = load_json(manifest_path)
    if manifest.get("run_id") != document["archive"]["run_id"]:
        found.append("archive run identity changed")
    if manifest.get("hash_policy") != HASH_POLICY:
        found.append("archive hash policy changed")
    listed = [item["path"] for item in manifest.get("files", [])]
    actual = {
        path.relative_to(archive).as_posix()
        for path in archive.rglob("*")
        if path.is_file() and path.name != "sha256sums.json"
    }
    if len(listed) != len(set(listed)) or set(listed) != actual:
        found.append("archive file set differs from checksum manifest")
    for item in manifest.get("files", []):
        path = archive / item["path"]
        if not path.is_file() or sha256(path) != item["sha256"]:
            found.append(f"archived artifact hash changed: {item['path']}")
    required = {
        "protocol.json", "build-provenance.json", "qualification-metrics.json",
        "local-sensitivity.json", "review-record.json", "qualification-report.md",
        "trajectories/baseline.csv", "trajectories/replay.csv",
        "trajectories/limit-zero-flux.csv", "trajectories/limit-zero-binding.csv",
        "trajectories/limit-constant-reservoir.csv",
    }
    if not required.issubset(actual):
        found.append("qualification archive is missing a mandatory artifact")
    if any("external" in path.lower() for path in actual):
        found.append("archive unexpectedly contains an external artifact")

    trajectories = {
        path.stem: read_trajectory(path, found)
        for path in sorted((archive / "trajectories").glob("*.csv"))
    }
    if len(trajectories) != document["execution"].get("trajectory_count"):
        found.append("trajectory count changed")
    if any(not rows for rows in trajectories.values()):
        return found
    maximum_balance = max(row["balance_error_mol"] for rows in trajectories.values() for row in rows)
    minimum_owner = min(row[name] for rows in trajectories.values() for row in rows for name in OWNER_COLUMNS)
    headline = document["headline_metrics"]
    if not close(maximum_balance, headline.get("maximum_balance_error_mol", -1.0)):
        found.append("maximum balance error was not reproduced")
    if not close(minimum_owner, headline.get("minimum_owner_amount_mol", -1.0)):
        found.append("minimum owner amount was not reproduced")
    replay_difference = max(
        abs(left[name] - right[name])
        for left, right in zip(trajectories["baseline"], trajectories["replay"], strict=True)
        for name in OWNER_COLUMNS
    )
    if replay_difference != headline.get("deterministic_replay_maximum_absolute_difference"):
        found.append("deterministic replay metric changed")
    reference = trajectories["time-step-0.5s"]
    time_error = final_error(trajectories["time-step-1s"], reference)
    if not close(time_error, headline.get("time_step_1s_relative_error", -1.0), 1.0e-9):
        found.append("time-step convergence metric changed")
    sync_error = final_error(trajectories["synchronization-30s"], trajectories["synchronization-15s"])
    if not close(sync_error, headline.get("synchronization_30s_relative_error", -1.0), 1.0e-9):
        found.append("synchronization convergence metric changed")

    metrics = load_json(archive / "qualification-metrics.json")
    if metrics.get("gates") != document["gates"]:
        found.append("archived and summary gates differ")
    gates = {item["id"]: item["status"] for item in document["gates"]}
    if gates != EXPECTED_GATE_STATUS:
        found.append("DCCQ gate status set changed or overclaims external evidence")
    increments = {item["id"]: item["status"] for item in document["increments"]}
    if increments != EXPECTED_INCREMENT_STATUS:
        found.append("DCCQ increment status set changed")
    sensitivity = load_json(archive / "local-sensitivity.json")
    if sensitivity.get("parameter_count") != 12 or sensitivity.get("joint_distribution_used") is not False:
        found.append("bounded local-sensitivity policy changed")
    if len(sensitivity.get("results", [])) != 12 or headline.get("local_sensitivity_comparisons") != 24:
        found.append("local-sensitivity result count changed")

    review = load_json(archive / "review-record.json")
    if review.get("machine_review") != "PASS" or review.get("documentation_review") != "PASS":
        found.append("machine or documentation review is incomplete")
    if review.get("external_human_review") != "BLOCKED_NO_ATTESTATION":
        found.append("external human-review blocker changed")
    decision = document["decision"]
    if decision.get("computational_qualification") != "PASS":
        found.append("computational qualification decision changed")
    if decision.get("independent_experimental_validation") != "BLOCKED":
        found.append("independent experimental evidence is overclaimed")
    if decision.get("biological_or_clinical_validation") != "NOT_ESTABLISHED":
        found.append("biological or clinical validity is overclaimed")
    for relative, token in DOCUMENTATION.items():
        path = root / relative
        if not path.is_file() or token not in path.read_text(encoding="utf-8"):
            found.append(f"documentation reconciliation missing: {relative}")
    return found


def main() -> int:
    document = load_json(RESULT)
    found = errors(document)
    if found:
        print("DCCQ-1 qualification result check: FAIL")
        for item in found:
            print(f"- {item}")
        return 1
    print(
        "DCCQ-1 qualification result check: PASS_WITH_BLOCKED_EXTERNAL_GATES "
        f"({document['execution']['trajectory_count']} trajectories, "
        f"{document['headline_metrics']['local_sensitivity_comparisons']} sensitivity comparisons)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
