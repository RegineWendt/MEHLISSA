# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Validate the auditable MEHLISSA Paper 1 release-candidate package."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import subprocess
import sys
from typing import Any
import zipfile

import jsonschema


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_CANDIDATE = (
    ROOT
    / "publication/paper1/release-candidates/paper1-platform-methods-rc1-20260903"
)


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"Expected a JSON object: {path}")
    return value


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def repository_file(relative: str) -> Path:
    candidate = PurePosixPath(relative)
    if candidate.is_absolute() or ".." in candidate.parts:
        raise ValueError(f"Unsafe repository path: {relative}")
    resolved = (ROOT / Path(*candidate.parts)).resolve()
    if ROOT not in resolved.parents:
        raise ValueError(f"Repository path escapes the root: {relative}")
    if not resolved.is_file():
        raise ValueError(f"Referenced file is absent: {relative}")
    return resolved


def verify_hashed_artifact(value: dict[str, Any]) -> None:
    path = repository_file(str(value["path"]))
    actual = sha256(path)
    if actual != value["sha256"]:
        raise ValueError(f"SHA-256 mismatch for {value['path']}: {actual}")


def verify_zip(path: Path, required_suffixes: tuple[str, ...]) -> None:
    if not zipfile.is_zipfile(path):
        raise ValueError(f"Invalid ZIP archive: {path}")
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        if not names:
            raise ValueError(f"Empty ZIP archive: {path}")
        for name in names:
            candidate = PurePosixPath(name)
            if candidate.is_absolute() or ".." in candidate.parts:
                raise ValueError(f"Unsafe ZIP member in {path}: {name}")
        for suffix in required_suffixes:
            if not any(name.endswith(suffix) for name in names):
                raise ValueError(f"ZIP {path} lacks required member *{suffix}")


def verify_claim_semantics(registry: dict[str, Any]) -> None:
    claims = registry["claims"]
    claim_ids = [claim["id"] for claim in claims]
    if len(claim_ids) != len(set(claim_ids)):
        raise ValueError("Claim IDs are not unique")
    prohibited_categories = {"biology", "clinical"}
    if any(
        claim["category"] in prohibited_categories and claim["status"] == "supported"
        for claim in claims
    ):
        raise ValueError("The candidate overstates biological or clinical support")
    if not any(claim["status"] == "not_supported" for claim in claims):
        raise ValueError("The registry has no explicit negative claim boundary")


def validate_candidate(candidate_directory: Path = DEFAULT_CANDIDATE) -> dict[str, Any]:
    candidate_directory = candidate_directory.resolve()
    manifest = load_json(candidate_directory / "release-candidate.json")
    registry = load_json(candidate_directory / "claim-to-artifact-registry.json")
    jsonschema.validate(
        manifest,
        load_json(ROOT / "data/schemas/paper1-release-candidate/1.0.0.schema.json"),
        format_checker=jsonschema.FormatChecker(),
    )
    jsonschema.validate(
        registry,
        load_json(ROOT / "data/schemas/paper1-claim-registry/1.0.0.schema.json"),
    )
    if manifest["candidate_id"] != registry["candidate_id"]:
        raise ValueError("Release candidate and claim registry IDs differ")

    verify_claim_semantics(registry)
    for claim in registry["claims"]:
        for artifact in claim["artifacts"]:
            repository_file(artifact)

    verify_hashed_artifact(manifest["source"] | {"path": manifest["source"]["archive_path"], "sha256": manifest["source"]["archive_sha256"]})
    verify_hashed_artifact(manifest["protocol"])
    verify_hashed_artifact(manifest["evidence_baseline"]["matrix"])
    verify_hashed_artifact(manifest["evidence_baseline"]["schema"])
    verify_hashed_artifact(manifest["claim_registry"])
    experiments = {entry["experiment_id"]: entry for entry in manifest["measurements"]}
    if set(experiments) != {
        "P1-E1-BODY-OBSERVATION",
        "P1-E2-M7-RESOURCE",
        "P1-E3-ACCESS-PARITY",
    }:
        raise ValueError("The release candidate does not contain exactly P1-E1 through P1-E3")
    for experiment in experiments.values():
        verify_hashed_artifact(experiment["report"])
        verify_hashed_artifact(experiment["raw_archive"])

    body = load_json(repository_file(experiments["P1-E1-BODY-OBSERVATION"]["report"]["path"]))
    if not (
        body.get("status") == "complete"
        and body.get("suitable_for_analysis") is True
        and body.get("correctness", {}).get("passed") is True
        and len(body.get("runs", [])) == 112
        and all(run.get("status") == "completed" for run in body["runs"])
    ):
        raise ValueError("P1-E1 is incomplete, unsuitable, or violates a core invariant")
    invalid_setup = load_json(candidate_directory / "measurements/P1-E1-invalid-setup-report.json")
    if not (
        invalid_setup.get("status") == "complete_with_failures"
        and invalid_setup.get("suitable_for_analysis") is False
        and sum(run.get("status") == "driver_failed" for run in invalid_setup.get("runs", [])) == 32
    ):
        raise ValueError("The retained P1-E1 setup deviation is not explicit")

    resource = load_json(repository_file(experiments["P1-E2-M7-RESOURCE"]["report"]["path"]))
    if not all(
        condition.get("attempt_count") == 3 and condition.get("deterministic_projection_identity") is True
        for condition in resource.get("conditions", {}).values()
    ):
        raise ValueError("P1-E2 lacks the declared repeated deterministic conditions")
    if not all(resource.get("negative_controls", {}).values()):
        raise ValueError("P1-E2 negative controls did not pass")

    parity = load_json(repository_file(experiments["P1-E3-ACCESS-PARITY"]["report"]["path"]))
    if not (
        parity.get("scientific_value_parity") is True
        and parity.get("authoritative_result_mutated") is False
        and parity.get("workbench", {}).get("source_integrity", {}).get("status") == "verified"
        and parity.get("workbench", {}).get("tampered_result_status", {}).get("status") == "attention"
    ):
        raise ValueError("P1-E3 access-path parity or tamper detection failed")

    verify_zip(repository_file(manifest["source"]["archive_path"]), ("README.md",))
    verify_zip(repository_file(experiments["P1-E1-BODY-OBSERVATION"]["raw_archive"]["path"]), ("campaign-report.json",))
    verify_zip(repository_file(experiments["P1-E2-M7-RESOURCE"]["raw_archive"]["path"]), ("analysis/summary.json", "attempts.jsonl"))
    verify_zip(repository_file(experiments["P1-E3-ACCESS-PARITY"]["raw_archive"]["path"]), ("access-parity-report.json",))

    source_commit = manifest["source"]["commit"]
    completed = subprocess.run(
        ["git", "cat-file", "-e", f"{source_commit}^{{commit}}"],
        cwd=ROOT,
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        raise ValueError(f"Source commit is not available: {source_commit}")

    sums_path = candidate_directory / "SHA256SUMS.json"
    sums = load_json(sums_path)
    if sums.get("candidate_id") != manifest["candidate_id"] or sums.get("algorithm") != "SHA-256":
        raise ValueError("SHA256SUMS identity is invalid")
    entries = sums.get("files", [])
    names = [entry.get("path") for entry in entries]
    if len(names) != len(set(names)):
        raise ValueError("SHA256SUMS contains duplicate paths")
    expected = {
        path.relative_to(candidate_directory).as_posix()
        for path in candidate_directory.rglob("*")
        if path.is_file() and path.name not in {"SHA256SUMS.json", "SHA256SUMS.json.license"}
    }
    if set(names) != expected:
        raise ValueError("SHA256SUMS does not cover the complete candidate directory")
    for entry in entries:
        path = candidate_directory / Path(*PurePosixPath(entry["path"]).parts)
        if sha256(path) != entry["sha256"] or path.stat().st_size != entry["bytes"]:
            raise ValueError(f"SHA256SUMS mismatch: {entry['path']}")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--candidate-directory", type=Path, default=DEFAULT_CANDIDATE)
    args = parser.parse_args()
    try:
        manifest = validate_candidate(args.candidate_directory)
    except (OSError, ValueError, json.JSONDecodeError, jsonschema.ValidationError) as error:
        print(f"paper1 release candidate: invalid: {error}", file=sys.stderr)
        return 1
    print(
        "paper1 release candidate: ok "
        f"({manifest['candidate_id']}, {len(manifest['measurements'])} experiments)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
