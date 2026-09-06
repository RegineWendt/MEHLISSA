# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Independently validate the checked-in BCQ-1.4 through BCQ-1.7 result."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


ROOT = Path(__file__).resolve().parents[1]
RESULT = ROOT / "data/qualification/biological-cell-model-qualification-result-v1.json"
SCHEMA = ROOT / "data/schemas/biological-cell-model-qualification-result/1.0.0.schema.json"
HASH_POLICY = "SHA-256 over Git-canonical LF bytes for text artifacts; binary bytes unchanged"
TEXT_SUFFIXES = {".csv", ".json", ".md", ".txt", ".cpp", ".hpp", ".xml"}
SPECIES = [
    "CD95", "FADD", "DISC", "p55free", "DISCp55", "p30", "p43", "p18",
    "p18inactive", "Bid", "tBid", "PrNES_mCherry", "PrNES", "mCherry",
    "PrER_mGFP", "PrER", "mGFP", "CD95L",
]
OBSERVABLES = ["PrER_mGFP", "PrNES_mCherry", "p43", "p18"]
PARAMETERS = [
    "kon_FADD", "koff_FADD", "kDISC", "kD216", "kD374trans_p55",
    "kD374trans_p43", "kdiss_p18", "kBid", "kD374probe", "KDR", "KDL",
]
EXPECTED_GATES = [
    "typed_adapter", "source_and_build_identity", "cross_engine_all_states",
    "deterministic_replay", "mehlissa_numerical_convergence",
    "invariants_and_nonnegativity", "same_family_structural_audit",
    "local_sensitivity_stability", "population_ensemble",
    "publication_curve_alignment", "external_human_review", "biological_qualification",
]
EXPECTED_REVIEW = [
    "source_identity", "licence", "code_to_equation_mapping", "typed_contract",
    "cross_engine_archive", "structural_scope", "population_scope", "claim_language",
    "user_guide", "roadmap", "requirements_matrix", "project_status",
    "publication_curve_alignment", "population_ensemble",
    "external_human_reviewer_attestation",
]
DOCUMENTATION_REQUIREMENTS = {
    "code_to_equation_mapping": (
        "docs/qualification/BCQ1_MEHLISSA_QUALIFICATION_RESULT.md",
        "Code-to-equation review",
    ),
    "user_guide": ("docs/USER_GUIDE.md", "BCQ-1.1 through BCQ-1.7"),
    "roadmap": ("docs/ROADMAP.md", "BCQ-1.1 through BCQ-1.7 complete"),
    "requirements_matrix": (
        "docs/requirements/TRACEABILITY_MATRIX.md",
        "34,596 all-state/time COPASI comparisons",
    ),
    "project_status": (
        "docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md",
        "BCQ-1.4 through BCQ-1.7 are now complete",
    ),
}


class QualificationResultError(ValueError):
    """Raised when the BCQ completion package is incomplete or inconsistent."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        return json.load(stream)


def canonical_bytes(path: Path) -> bytes:
    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES:
        return data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
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


def close(left: float, right: float) -> bool:
    return math.isclose(left, right, rel_tol=1e-12, abs_tol=1e-15)


def read_trajectory(path: Path, found: list[str]) -> list[dict[str, float]]:
    try:
        with path.open(encoding="utf-8-sig", newline="") as stream:
            reader = csv.DictReader(stream)
            if reader.fieldnames != ["model_time", *SPECIES]:
                found.append(f"trajectory columns changed: {path.name}")
                return []
            rows = [{key: float(value) for key, value in row.items()} for row in reader]
    except (OSError, TypeError, ValueError) as error:
        found.append(f"cannot parse trajectory {path.name}: {error}")
        return []
    if len(rows) != 961:
        found.append(f"trajectory row count changed: {path.name}")
    for index, row in enumerate(rows):
        if not close(row["model_time"], index * 0.25):
            found.append(f"trajectory grid changed: {path.name} row {index}")
            break
        if not all(math.isfinite(value) for value in row.values()):
            found.append(f"trajectory contains non-finite value: {path.name} row {index}")
            break
    return rows


def comparison(
    actual: list[dict[str, float]],
    reference: list[dict[str, float]],
    absolute_floor: float,
    scale_relative: float,
) -> tuple[float, float]:
    maximum_absolute = 0.0
    maximum_fraction = 0.0
    for actual_row, reference_row in zip(actual, reference, strict=True):
        for species in SPECIES:
            difference = abs(actual_row[species] - reference_row[species])
            limit = absolute_floor + scale_relative * max(1.0, abs(reference_row[species]))
            maximum_absolute = max(maximum_absolute, difference)
            maximum_fraction = max(maximum_fraction, difference / limit)
    return maximum_absolute, maximum_fraction


def invariant_metrics(rows: list[dict[str, float]], case: str) -> tuple[float, float, float]:
    expected = {
        "523": [93.0, 155.0, 236.0, 973.0, 973.0, 5178.0, 5178.0, 116.0, 16.6],
        "524": [90.0, 127.0, 224.0, 1909.0, 1909.0, 3316.0, 3316.0, 12.0, 16.6],
    }[case]
    invariants = [
        ["FADD", "DISC", "DISCp55", "p30", "p43"],
        ["p55free", "DISCp55", "p30", "p43", "p18", "p18inactive"],
        ["Bid", "tBid"], ["PrNES_mCherry", "PrNES"],
        ["PrNES_mCherry", "mCherry"], ["PrER_mGFP", "PrER"],
        ["PrER_mGFP", "mGFP"], ["CD95"], ["CD95L"],
    ]
    maximum_residual = max(
        abs(sum(row[member] for member in members) - target)
        for row in rows
        for members, target in zip(invariants, expected, strict=True)
    )
    minimum = min(row[species] for row in rows for species in SPECIES)
    reporter_increase = max(
        max(rows[index + 1][reporter] - rows[index][reporter] for index in range(960))
        for reporter in ("PrER_mGFP", "PrNES_mCherry")
    )
    return maximum_residual, minimum, reporter_increase


def verify_archive(
    document: dict[str, Any], root: Path, found: list[str]
) -> tuple[Path | None, set[str]]:
    archive = document["archive"]
    archive_dir = repository_path(root, archive["path"], found)
    manifest_path = repository_path(root, archive["checksum_manifest"]["path"], found)
    if archive_dir is None or not archive_dir.is_dir():
        found.append("authoritative qualification archive is missing")
        return None, set()
    if manifest_path is None or not manifest_path.is_file():
        found.append("qualification archive checksum manifest is missing")
        return archive_dir, set()
    if archive["hash_canonicalization"] != HASH_POLICY:
        found.append("archive hash-canonicalization policy changed")
    if sha256(manifest_path) != archive["checksum_manifest"]["sha256"]:
        found.append("qualification checksum-manifest hash changed")
        return archive_dir, set()
    manifest = load_json(manifest_path)
    if manifest.get("run_id") != archive["run_id"] or manifest.get("hash_policy") != HASH_POLICY:
        found.append("qualification checksum-manifest identity changed")
    entries = manifest.get("files", [])
    listed = [item.get("path") for item in entries]
    actual = {
        path.relative_to(archive_dir).as_posix()
        for path in archive_dir.rglob("*")
        if path.is_file() and path.name != "sha256sums.json"
    }
    if len(listed) != len(set(listed)) or set(listed) != actual:
        found.append("qualification archive file set differs from checksum manifest")
    for entry in entries:
        path = repository_path(archive_dir, entry["path"], found)
        if path is None or not path.is_file() or sha256(path) != entry["sha256"]:
            found.append(f"qualification archive file hash changed: {entry['path']}")
    required = {
        "protocol.json", "build-provenance.json", "local-sensitivity.json",
        "qualification-metrics.json", "review-record.json", "qualification-report.md",
    }
    required.update(
        f"trajectories/BIOMD0000000{case}-mehlissa-{kind}.csv"
        for case in ("523", "524")
        for kind in ("primary", "replay", "tightened")
    )
    if not required.issubset(actual):
        found.append("qualification archive is missing a required artifact")
    if any(path.lower().endswith((".xml", ".sbml")) for path in actual):
        found.append("external SBML was bundled despite the recorded policy")
    return archive_dir, actual


def errors(document: dict[str, Any], root: Path = ROOT) -> list[str]:
    found: list[str] = []
    validator = Draft202012Validator(load_json(SCHEMA), format_checker=FormatChecker())
    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path) or "<root>"
        found.append(f"{location}: {error.message}")
    if found:
        return found

    protocol_path = repository_path(root, document["protocol"]["path"], found)
    parent_path = repository_path(root, document["parent_result"]["path"], found)
    if protocol_path is None or not protocol_path.is_file() or sha256(protocol_path) != document["protocol"]["sha256"]:
        found.append("integration protocol identity changed")
        return found
    if parent_path is None or not parent_path.is_file() or sha256(parent_path) != document["parent_result"]["sha256"]:
        found.append("parent COPASI result identity changed")
        return found
    protocol = load_json(protocol_path)
    for frozen in protocol["implementation"].values():
        if not isinstance(frozen, dict) or "path" not in frozen:
            continue
        path = repository_path(root, frozen["path"], found)
        if path is None or not path.is_file() or sha256(path) != frozen["sha256"]:
            found.append(f"frozen implementation changed: {frozen.get('path', '<unknown>')}")

    archive_dir, actual_files = verify_archive(document, root, found)
    if archive_dir is None or not actual_files:
        return found
    if sha256(archive_dir / "protocol.json") != document["protocol"]["sha256"]:
        found.append("archived protocol copy changed")
    provenance = load_json(archive_dir / "build-provenance.json")
    if provenance.get("git_commit") != document["result"]["baseline_commit"]:
        found.append("build provenance baseline commit changed")
    if provenance.get("protocol_sha256") != document["protocol"]["sha256"]:
        found.append("build provenance protocol identity changed")

    metrics = load_json(archive_dir / "qualification-metrics.json")
    trajectories: dict[tuple[str, str], list[dict[str, float]]] = {}
    maximum_cross_absolute = 0.0
    maximum_cross_fraction = 0.0
    maximum_convergence_fraction = 0.0
    maximum_invariant = 0.0
    minimum_state = math.inf
    for case in ("523", "524"):
        accession = f"BIOMD0000000{case}"
        for kind in ("primary", "replay", "tightened"):
            path = archive_dir / "trajectories" / f"{accession}-mehlissa-{kind}.csv"
            trajectories[(case, kind)] = read_trajectory(path, found)
        primary = trajectories[(case, "primary")]
        replay = trajectories[(case, "replay")]
        tightened = trajectories[(case, "tightened")]
        if not all(len(rows) == 961 for rows in (primary, replay, tightened)):
            continue
        reference_record = next(
            item for item in protocol["cross_engine_acceptance"]["reference_files"]
            if item["accession"] == accession
        )
        reference_path = repository_path(root, reference_record["path"], found)
        if reference_path is None or not reference_path.is_file() or sha256(reference_path) != reference_record["sha256"]:
            found.append(f"COPASI reference identity changed: {accession}")
            continue
        reference = read_trajectory(reference_path, found)
        cross_rule = protocol["cross_engine_acceptance"]["all_state_tolerance"]
        absolute, fraction = comparison(
            primary, reference, cross_rule["absolute_floor"], cross_rule["scale_relative"]
        )
        maximum_cross_absolute = max(maximum_cross_absolute, absolute)
        maximum_cross_fraction = max(maximum_cross_fraction, fraction)
        convergence_rule = protocol["cross_engine_acceptance"]["mehlissa_convergence_tolerance"]
        _, convergence = comparison(
            primary, tightened, convergence_rule["absolute_floor"],
            convergence_rule["scale_relative"],
        )
        maximum_convergence_fraction = max(maximum_convergence_fraction, convergence)
        if canonical_bytes(
            archive_dir / "trajectories" / f"{accession}-mehlissa-primary.csv"
        ) != canonical_bytes(
            archive_dir / "trajectories" / f"{accession}-mehlissa-replay.csv"
        ):
            found.append(f"deterministic replay changed: {accession}")
        invariant, minimum, reporter_increase = invariant_metrics(primary, case)
        maximum_invariant = max(maximum_invariant, invariant)
        minimum_state = min(minimum_state, minimum)
        if invariant > 1e-8 or minimum < -1e-10 or reporter_increase > 1e-8:
            found.append(f"state-integrity gate failed: {accession}")
        recorded = next(item for item in metrics["cases"] if item["accession"] == accession)
        if recorded["integrity"].get("invariant_count") != 9:
            found.append(f"archive does not report all nine invariants: {accession}")
        if not close(recorded["cross_engine"]["maximum_fraction_of_allowed_difference"], fraction):
            found.append(f"recorded cross-engine metric changed: {accession}")
        if not close(recorded["mehlissa_convergence"]["maximum_fraction_of_allowed_difference"], convergence):
            found.append(f"recorded convergence metric changed: {accession}")

    sensitivity = load_json(archive_dir / "local-sensitivity.json")
    if sensitivity.get("population_interpretation") != "forbidden":
        found.append("local sensitivity was reclassified as population evidence")
    maximum_sensitivity = 0.0
    comparison_count = 0
    if [case.get("accession") for case in sensitivity.get("cases", [])] != [
        "BIOMD0000000523", "BIOMD0000000524"
    ]:
        found.append("local-sensitivity case identity changed")
    for case in sensitivity.get("cases", []):
        entries = case.get("entries", [])
        if [entry.get("parameter") for entry in entries] != PARAMETERS:
            found.append(f"local-sensitivity parameter identity changed: {case.get('accession')}")
            continue
        for entry in entries:
            estimates = entry["normalized_central_sensitivity"]
            if set(estimates) != {"0.01", "0.005"}:
                found.append("local-sensitivity step set changed")
                continue
            for observable in OBSERVABLES:
                coarse = float(estimates["0.01"][observable])
                fine = float(estimates["0.005"][observable])
                calculated = (
                    0.0 if max(abs(coarse), abs(fine)) <= 1e-8
                    else abs(coarse - fine) / max(abs(fine), 1e-12)
                )
                recorded = entry["step_stability"][observable]
                if not close(calculated, float(recorded["relative_difference"])):
                    found.append("local-sensitivity stability metric changed")
                if recorded["status"] == "FAIL" or calculated > 0.05:
                    found.append("local-sensitivity stability gate failed")
                maximum_sensitivity = max(maximum_sensitivity, calculated)
                comparison_count += 1

    structural = metrics.get("structural_sensitivity", [])
    expected_structural = protocol["structural_sensitivity"]["companions"]
    if [item.get("accession") for item in structural] != [item["accession"] for item in expected_structural]:
        found.append("structural companion identity changed")
    for observed, expected in zip(structural, expected_structural):
        expected_shape = {key: expected[key] for key in ("species", "reactions", "global_parameters", "assignment_rules")}
        if observed.get("observed_structure") != expected_shape or observed.get("status") != "PASS":
            found.append(f"structural companion audit changed: {expected['accession']}")
        if observed.get("role") != "same-publication-structural-sensitivity-only":
            found.append(f"structural companion role was overclaimed: {expected['accession']}")

    calculated_headline = {
        "maximum_cross_engine_absolute_difference": maximum_cross_absolute,
        "maximum_cross_engine_fraction_of_limit": maximum_cross_fraction,
        "maximum_mehlissa_convergence_fraction_of_limit": maximum_convergence_fraction,
        "maximum_invariant_residual": maximum_invariant,
        "minimum_state": minimum_state,
        "maximum_sensitivity_step_disagreement": maximum_sensitivity,
    }
    for key, value in calculated_headline.items():
        if not close(float(document["headline_metrics"][key]), value):
            found.append(f"headline metric disagrees with raw archive: {key}")
    if comparison_count != 88 or document["headline_metrics"]["sensitivity_step_comparisons"] != 88:
        found.append("sensitivity comparison count changed")

    gates = document["gates"]
    if [gate["id"] for gate in gates] != EXPECTED_GATES:
        found.append("qualification gate identity or order changed")
    if any(gate["status"] != "PASS" for gate in gates[:8]):
        found.append("an unblocked computational gate is not PASS")
    if [gate["status"] for gate in gates[8:]] != ["BLOCKED", "BLOCKED", "BLOCKED", "NOT_ESTABLISHED"]:
        found.append("a blocked biological-evidence gate was promoted")
    if [(gate["id"], gate["status"]) for gate in metrics["gates"]] != [
        (gate["id"], gate["status"]) for gate in gates
    ]:
        found.append("archive gates disagree with the result decision")

    review = load_json(archive_dir / "review-record.json")
    dimensions = review.get("dimensions", [])
    if [item.get("dimension") for item in dimensions] != EXPECTED_REVIEW:
        found.append("review dimension identity or order changed")
    if any(item.get("status") != "PASS" for item in dimensions[:12]):
        found.append("an unblocked review dimension is not PASS")
    if [item.get("status") for item in dimensions[12:]] != ["BLOCKED"] * 3:
        found.append("a blocked review dimension was promoted")
    if review.get("negative_controls_passed") != 12 or review.get("negative_controls_total") != 12:
        found.append("negative-control count changed")
    for dimension, (relative, token) in DOCUMENTATION_REQUIREMENTS.items():
        path = repository_path(root, relative, found)
        if path is None or not path.is_file() or token not in path.read_text(encoding="utf-8"):
            found.append(f"documentation completion evidence changed: {dimension}")
        reviewed = next((item for item in dimensions if item.get("dimension") == dimension), {})
        if reviewed.get("evidence") != relative:
            found.append(f"review evidence path changed: {dimension}")

    attempts = document["attempt_history"]
    if [item["authoritative"] for item in attempts] != [False, False, True]:
        found.append("attempt authority history changed")
    for attempt in attempts[:-1]:
        path = repository_path(root, attempt["path"], found)
        if path is None or not path.is_dir():
            found.append(f"superseded archive is missing: {attempt['run_id']}")
            continue
        supersession_path = path / "supersession.json"
        if not supersession_path.is_file():
            found.append(f"supersession record is missing: {attempt['run_id']}")
            continue
        supersession = load_json(supersession_path)
        if supersession.get("status") != attempt["status"] or supersession.get("authoritative"):
            found.append(f"supersession decision changed: {attempt['run_id']}")

    policy = protocol["review_and_claim_policy"]
    decision = document["decision"]
    if decision["allowed_claim"] != policy["allowed_if_unblocked_computational_gates_pass"]:
        found.append("allowed claim differs from the prospective protocol")
    if decision["forbidden_claims"] != policy["forbidden_claims"]:
        found.append("forbidden claims differ from the prospective protocol")
    if review.get("allowed_claim") != decision["allowed_claim"] or review.get("forbidden_claims") != decision["forbidden_claims"]:
        found.append("archive claim language differs from the result decision")
    return found


def validate(path: Path = RESULT, root: Path = ROOT) -> dict[str, Any]:
    document = load_json(path)
    found = errors(document, root)
    if found:
        raise QualificationResultError("; ".join(found))
    return document


def main() -> int:
    try:
        document = validate()
    except (OSError, ValueError) as error:
        print(f"BCQ-1.4-1.7 qualification result: FAILED\n{error}")
        return 1
    print(
        "BCQ-1.4-1.7 qualification result: PASS_WITH_BLOCKED_BIOLOGICAL_GATES "
        f"({document['archive']['run_id']})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
