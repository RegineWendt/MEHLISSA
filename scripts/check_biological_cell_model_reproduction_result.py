# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Independently validate the checked-in BCQ-1.3 result and run archive."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
RESULT = ROOT / "data/qualification/biological-cell-model-reproduction-result-v1.json"
SCHEMA = ROOT / "data/schemas/biological-cell-model-reproduction-result/1.0.0.schema.json"
EXPECTED_BASELINE = "2fe6a531ada559c8852a9696b4a680244c3aea41"
EXPECTED_EXECUTION_HASH = "5dd09984d838c0a4237d9d100256a91a4e4db5f4473ac0601749db105912d5dd"
EXPECTED_SOLVER_BINARY = "f311e16e4b94aea86b972c956e55fdc0dc275fbb69937ae90b73876e040f128d"
SPECIES = [
    "CD95", "FADD", "DISC", "p55free", "DISCp55", "p30", "p43", "p18",
    "p18inactive", "Bid", "tBid", "PrNES_mCherry", "PrNES", "mCherry",
    "PrER_mGFP", "PrER", "mGFP", "CD95L",
]
EXPECTED_GATES = [
    "source_identity", "import_structure", "complete_output", "initial_state",
    "deterministic_replay", "numerical_convergence", "conservation",
    "nonnegativity", "reporter_direction", "publication_alignment",
]


class ReproductionResultError(ValueError):
    """Raised when the BCQ-1.3 evidence package is incomplete or inconsistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def repository_path(root: Path, relative: str, errors: list[str]) -> Path | None:
    path = (root / relative).resolve()
    try:
        path.relative_to(root.resolve())
    except ValueError:
        errors.append(f"repository path escapes root: {relative}")
        return None
    return path


def close(left: float, right: float, tolerance: float = 1e-18) -> bool:
    return math.isclose(left, right, rel_tol=0.0, abs_tol=tolerance)


def read_trajectory(path: Path, errors: list[str]) -> list[dict[str, float]]:
    try:
        with path.open(newline="", encoding="utf-8-sig") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != ["model_time", *SPECIES]:
                errors.append(f"trajectory columns changed: {path.name}")
                return []
            rows = [{key: float(value) for key, value in row.items()} for row in reader]
    except (OSError, ValueError, TypeError) as error:
        errors.append(f"cannot parse trajectory {path.name}: {error}")
        return []
    if len(rows) != 961:
        errors.append(f"trajectory row count changed: {path.name}")
        return rows
    for index, row in enumerate(rows):
        if not close(row["model_time"], index * 0.25, 1e-12):
            errors.append(f"trajectory grid changed: {path.name} row {index}")
            break
        if not all(math.isfinite(value) for value in row.values()):
            errors.append(f"trajectory contains non-finite value: {path.name} row {index}")
            break
    return rows


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    result: list[str] = []
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        result.append(f"{location}: {error.message}")
    if result:
        return result

    if document["result"]["baseline_commit"] != EXPECTED_BASELINE:
        result.append("prospective amendment baseline commit changed")

    lineage = document["protocol_lineage"]
    lineage_documents: dict[str, dict[str, Any]] = {}
    for label in ("base_protocol", "prospective_amendment"):
        asset = lineage[label]
        path = repository_path(root, asset["path"], result)
        if path is None or not path.is_file():
            result.append(f"{label} is missing")
            continue
        if sha256(path) != asset["sha256"]:
            result.append(f"{label} hash changed")
            continue
        lineage_documents[label] = load_json(path)
    if len(lineage_documents) != 2:
        return result
    identity = hashlib.sha256(
        f"{lineage['base_protocol']['sha256']}:{lineage['prospective_amendment']['sha256']}".encode("ascii")
    ).hexdigest()
    if identity != lineage["execution_protocol_sha256"] or identity != EXPECTED_EXECUTION_HASH:
        result.append("combined execution protocol identity changed")

    base = lineage_documents["base_protocol"]
    amendment = lineage_documents["prospective_amendment"]
    if amendment["amendment"]["base_protocol"] != lineage["base_protocol"]:
        result.append("amendment no longer binds the recorded base protocol")
    replay_rule = amendment["changes"]["deterministic_replay"]
    if replay_rule["absolute_tolerance"] != 1e-12 or replay_rule["relative_tolerance"] != 1e-12:
        result.append("prospective replay-equivalence tolerance changed")
    if amendment["changes"]["execution_timeout"]["seconds_per_solver_process"] != 120:
        result.append("prospective solver timeout changed")

    archive = document["archive"]
    archive_dir = repository_path(root, archive["path"], result)
    manifest_path = repository_path(root, archive["checksum_manifest"]["path"], result)
    if archive_dir is None or not archive_dir.is_dir():
        result.append("authoritative result archive is missing")
        return result
    if manifest_path is None or not manifest_path.is_file():
        result.append("archive checksum manifest is missing")
        return result
    if sha256(manifest_path) != archive["checksum_manifest"]["sha256"]:
        result.append("archive checksum-manifest hash changed")
        return result

    manifest = load_json(manifest_path)
    if manifest.get("run_id") != archive["run_id"]:
        result.append("archive run identifier disagrees with checksum manifest")
    entries = manifest.get("files", [])
    listed = [item.get("path") for item in entries]
    if len(listed) != len(set(listed)):
        result.append("archive checksum manifest contains duplicate paths")
    actual_files = {
        path.relative_to(archive_dir).as_posix()
        for path in archive_dir.rglob("*")
        if path.is_file() and path.name != "sha256sums.json"
    }
    if set(listed) != actual_files:
        result.append("archive file set differs from checksum manifest")
    for entry in entries:
        path = repository_path(archive_dir, entry["path"], result)
        if path is None or not path.is_file() or sha256(path) != entry["sha256"]:
            result.append(f"archive file hash changed: {entry['path']}")

    required = {
        "protocol.json", "protocol-base-v1.json", "protocol-amendment-v1.1.json",
        "source-manifest.json", "solver-provenance.json", "reproduction-metrics.json",
        "validation-report.json", "validation-report.md",
    }
    required.update(
        f"trajectories/{accession}-{kind}.csv"
        for accession in ("BIOMD0000000523", "BIOMD0000000524")
        for kind in ("primary", "replay", "tightened")
    )
    if not required.issubset(actual_files):
        result.append("archive is missing a required BCQ-1.3 artifact")
    if any(path.lower().endswith((".xml", ".sbml")) for path in actual_files):
        result.append("external SBML source was bundled despite the recorded policy")
    if sha256(archive_dir / "protocol.json") != lineage["base_protocol"]["sha256"]:
        result.append("archive protocol.json is not the frozen base protocol")
    if sha256(archive_dir / "protocol-base-v1.json") != lineage["base_protocol"]["sha256"]:
        result.append("archive base-protocol lineage copy changed")
    if sha256(archive_dir / "protocol-amendment-v1.1.json") != lineage["prospective_amendment"]["sha256"]:
        result.append("archive amendment lineage copy changed")

    solver = load_json(archive_dir / "solver-provenance.json")
    execution = document["execution"]
    for field in ("name", "version", "build", "method", "binary_sha256"):
        record_field = "solver" if field == "name" else field
        if solver[field] != execution[record_field]:
            result.append(f"solver provenance disagrees on {field}")
    if solver["binary_sha256"] != EXPECTED_SOLVER_BINARY:
        result.append("COPASI binary identity changed")
    if solver["optimization_or_parameter_estimation_enabled"] or solver["reduced_model"]:
        result.append("solver provenance enables a forbidden task or reduced model")
    unit_note = solver["copasi_import_default_units"]["interpretation"].lower()
    if not all(phrase in unit_note for phrase in ("import defaults only", "unresolved-model-native", "omit explicit")):
        result.append("solver import defaults are presented as source units")

    source_manifest = load_json(archive_dir / "source-manifest.json")
    if source_manifest["external_source_files_bundled"]:
        result.append("source manifest claims external source bundling")
    expected_sources = {item["accession"]: item for item in base["source_artifacts"]}
    observed_sources = {item["accession"]: item for item in source_manifest["sources"]}
    if set(observed_sources) != set(expected_sources):
        result.append("source-manifest accession set changed")
    for accession, expected in expected_sources.items():
        observed = observed_sources.get(accession, {})
        for field in ("role", "source_commit", "sha256", "license"):
            if observed.get(field) != expected[field]:
                result.append(f"source-manifest {accession} {field} changed")

    trajectories: dict[tuple[str, str], list[dict[str, float]]] = {}
    for accession in expected_sources:
        for kind in ("primary", "replay", "tightened"):
            trajectories[(accession, kind)] = read_trajectory(
                archive_dir / "trajectories" / f"{accession}-{kind}.csv", result
            )

    maximum_replay_fraction = 0.0
    maximum_convergence_fraction = 0.0
    maximum_invariant_residual = 0.0
    minimum_species_value = math.inf
    observables = [item["id"] for item in base["observables"]]
    for accession, artifact in expected_sources.items():
        primary = trajectories[(accession, "primary")]
        replay = trajectories[(accession, "replay")]
        tightened = trajectories[(accession, "tightened")]
        if not all(len(rows) == 961 for rows in (primary, replay, tightened)):
            continue
        for rows in (primary, replay, tightened):
            for species, expected in artifact["initial_values"].items():
                if not close(rows[0][species], expected, 1e-12):
                    result.append(f"initial value changed: {accession} {species}")
            minimum_species_value = min(
                minimum_species_value,
                min(row[species] for row in rows for species in SPECIES),
            )
            nonnegative_limit = -1e-10 * max(
                1.0, max(abs(value) for value in artifact["initial_values"].values())
            )
            if minimum_species_value < nonnegative_limit:
                result.append(f"material negative state detected: {accession}")
            for invariant in base["invariants"]:
                expected_total = invariant["expected_initial_total_by_accession"][accession]
                residual = max(
                    abs(sum(row[species] for species in invariant["species"]) - expected_total)
                    for row in rows
                )
                maximum_invariant_residual = max(maximum_invariant_residual, residual)
                limit = 1e-10 + 1e-8 * max(1.0, abs(expected_total))
                if residual > limit:
                    result.append(f"conservation limit exceeded: {accession} {invariant['id']}")
            for reporter in ("PrER_mGFP", "PrNES_mCherry"):
                margin = 1e-10 + 1e-8 * max(1.0, abs(artifact["initial_values"][reporter]))
                if max(rows[index][reporter] - rows[index - 1][reporter] for index in range(1, len(rows))) > margin:
                    result.append(f"reporter direction changed: {accession} {reporter}")
        for first, second in zip(primary, replay):
            for column in ("model_time", *SPECIES):
                difference = abs(first[column] - second[column])
                limit = replay_rule["absolute_tolerance"] + replay_rule["relative_tolerance"] * max(1.0, abs(second[column]))
                maximum_replay_fraction = max(maximum_replay_fraction, difference / limit)
        if maximum_replay_fraction > 1.0:
            result.append(f"replay equivalence limit exceeded: {accession}")
        for first, second in zip(primary, tightened):
            for observable in observables:
                difference = abs(first[observable] - second[observable])
                limit = 1e-8 + 1e-6 * max(1.0, abs(second[observable]))
                maximum_convergence_fraction = max(maximum_convergence_fraction, difference / limit)
        if maximum_convergence_fraction > 1.0:
            result.append(f"tightened-solver convergence limit exceeded: {accession}")

    metrics = load_json(archive_dir / "reproduction-metrics.json")
    report = load_json(archive_dir / "validation-report.json")
    if metrics["execution_protocol_sha256"] != EXPECTED_EXECUTION_HASH:
        result.append("metrics execution protocol identity changed")
    if report["status"] != "PASS_WITH_BLOCKED_PUBLICATION_ALIGNMENT":
        result.append("validation report decision changed")
    if report["negative_controls_passed"] != 10:
        result.append("validation report negative-control count changed")
    negative_controls = metrics["negative_controls"]
    if [item["id"] for item in negative_controls] != [f"BCQ-NC-{index:02d}" for index in range(1, 11)]:
        result.append("negative-control identity or order changed")
    if any(item["status"] != "PASS" or not item["observed_rejection"] for item in negative_controls):
        result.append("a negative control lacks its expected rejection")
    if len(metrics["configuration_records"]) != 6:
        result.append("configuration record count changed")

    headline = document["headline_metrics"]
    calculated = {
        "maximum_replay_fraction_of_limit": maximum_replay_fraction,
        "maximum_convergence_fraction_of_limit": maximum_convergence_fraction,
        "maximum_invariant_residual": maximum_invariant_residual,
        "minimum_species_value": minimum_species_value,
    }
    for key, value in calculated.items():
        if not close(headline[key], value):
            result.append(f"headline metric disagrees with raw trajectories: {key}")

    gates = document["gates"]
    if [item["id"] for item in gates] != EXPECTED_GATES:
        result.append("result gate identity or order changed")
    if any(item["status"] != "PASS" for item in gates[:-1]):
        result.append("an unblocked computational gate is not PASS")
    if gates[-1]["status"] != "BLOCKED" or "machine-readable" not in gates[-1].get("reason", ""):
        result.append("publication-alignment block changed")
    report_gates = [(item["id"], item["status"]) for item in report["computational_gates"]]
    if report_gates != [(item["id"], item["status"]) for item in gates]:
        result.append("summary gates disagree with the archived validation report")

    history = document["attempt_history"]
    if sum(item["authoritative"] for item in history) != 1 or not history[-1]["authoritative"]:
        result.append("attempt history must name exactly the final run as authoritative")
    for attempt in history:
        path = root / "results/bcq1/kallenberger-minimal" / attempt["run_id"]
        if not path.is_dir() or not any(item.is_file() for item in path.rglob("*")):
            result.append(f"retained attempt archive is missing: {attempt['run_id']}")

    decision_text = json.dumps(document["decision"]).lower()
    for phrase in (
        "publication curve", "held-out cell population", "biological qualification",
        "patient-level", "clinical validity", "software_test_surrogate",
    ):
        if phrase not in decision_text:
            result.append(f"required result limitation is absent: {phrase}")
    return result


def validate(path: Path = RESULT, root: Path = ROOT) -> dict[str, Any]:
    document = load_json(path)
    found = errors(document, root)
    if found:
        raise ReproductionResultError("\n".join(found))
    return document


def main() -> int:
    try:
        document = validate()
        print(
            "BCQ-1.3 biological cell-model reproduction result: ok "
            f"({document['execution']['run_count']} runs, "
            f"{document['execution']['points_per_run']} points each, "
            "9 computational gates PASS, publication alignment BLOCKED, "
            "M5 remains software_test_surrogate)"
        )
    except (OSError, csv.Error, json.JSONDecodeError, KeyError, ReproductionResultError) as error:
        print(f"BCQ-1.3 biological cell-model reproduction result: FAILED\n{error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
